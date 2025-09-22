#include "make_puzzle_image.h"
#include "ui_make_puzzle_image.h"
#include "puzzle_piece_save.h"
#include <QGraphicsScene>
#include <QDir>
#include <QPixmap>
#include <QDebug>

#include <opencv2/opencv.hpp>
#include <filesystem>

makePuzzleImage::makePuzzleImage(QWidget *parent)
    : QWidget(parent)
    , make_puzzle_image_ui(new Ui::make_puzzle_image)
{
    make_puzzle_image_ui->setupUi(this);
}

makePuzzleImage::~makePuzzleImage()
{
    delete make_puzzle_image_ui;
}

void makePuzzleImage::on_make_puzzle_btn_clicked()
{
    if (!m_scene || !m_bgItem || !m_maskItem) {
        qDebug() << "저장 실패: scene/bg/mask 준비 안됨";
        return;
    }

    // 1) 저장 경로 준비
    const QString saveDir = QCoreApplication::applicationDirPath() + "/images/capture_image";
    QDir().mkpath(saveDir);
    const QString outPath = saveDir + "/puzzle_image.png";

    // 2) 마스크가 차지하는 씬 영역만큼 캔버스 만들기 (ARGB, 투명 배경)
    QRectF srcRect = m_maskItem->sceneBoundingRect();             // 씬 좌표
    QSize  outSize = srcRect.size().toSize();
    if (outSize.isEmpty()) {
        qDebug() << "저장 실패: 마스크 크기 0";
        return;
    }

    QImage out(outSize, QImage::Format_ARGB32_Premultiplied);
    out.fill(Qt::transparent);

    // 3) 장면을 해당 영역으로 렌더(= 배경+마스크 합쳐서 잘라내기)
    QPainter p(&out);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // dest: (0,0)-(outSize), source: srcRect(씬 좌표)
    m_scene->render(&p, QRectF(QPointF(0,0), outSize), srcRect);
    p.end();

    // 4) 파일 저장
    if (out.save(outPath)) {
        qDebug() << "퍼즐 이미지 저장 성공:" << outPath;
    } else {
        qDebug() << "퍼즐 이미지 저장 실패:" << outPath;
    }

    // ⭐ 수정: OpenCV 기반 퍼즐 분할 추가
    {
        cv::Mat cvCapture = cv::imread(m_capturePath.toStdString());
        if (cvCapture.empty()) {
            qWarning() << "캡처 이미지 OpenCV 로드 실패:" << m_capturePath;
            return;
        }

        // 드래그된 마스크 위치
        QRectF maskRect = m_maskItem->sceneBoundingRect();
        cv::Rect roi(maskRect.x(), maskRect.y(), maskRect.width(), maskRect.height());

        roi.x = std::max(0, roi.x);
        roi.y = std::max(0, roi.y);
        roi.width  = std::min(roi.width,  cvCapture.cols - roi.x);
        roi.height = std::min(roi.height, cvCapture.rows - roi.y);
        if (roi.width <= 0 || roi.height <= 0) {
            qWarning() << "ROI 유효 크기 없음!";
            return;
        }

        cv::Mat roiImg = cvCapture(roi).clone();

        // 퍼즐 마스크 로드
        cv::Mat mask;
        if (m_puzzleType == 5)
            mask = cv::imread("./images/puzzle_mask_5x5.png", cv::IMREAD_GRAYSCALE);
        else
            mask = cv::imread("./images/puzzle_mask_8x8.png", cv::IMREAD_GRAYSCALE);

        if (mask.empty()) {
            qWarning() << "퍼즐 마스크 OpenCV 로드 실패!";
            return;
        }

        // ROI 크기에 맞게 마스크 리사이즈
        cv::Mat maskResized;
        cv::resize(mask, maskResized, roiImg.size());

        // ⭐ 여기서 puzzle_piece_save.cpp에 있던 makePuzzlePieces() 사용
        auto pieces = puzzle_piece_save::makePuzzlePieces(roiImg, maskResized);

        // 저장 경로 정리
        std::filesystem::remove_all("./images/piece_image/");
        std::filesystem::create_directories("./images/piece_image/");

        // 퍼즐 조각 저장 (pieces[i].img는 이미 RGBA)
        for (size_t i = 0; i < pieces.size(); ++i) {
            std::string pieceName = "./images/piece_image/piece_" + std::to_string(i) + ".png";
            cv::imwrite(pieceName, pieces[i].img);   // ← 그대로 저장하면 투명 배경 유지
        }

        qDebug() << "퍼즐 조각 저장 완료!";
    }
    // ⭐ 수정 끝

    // 기존 동작 유지(필요 시 페이지 전환)
    emit showPlayPage();
}



void makePuzzleImage::loadCapturedImage()
{
    m_scene = new QGraphicsScene(this);
    auto *view  = make_puzzle_image_ui->capture_image_graphicsView;
    view->setScene(m_scene);
    view->setDragMode(QGraphicsView::NoDrag);
    view->setInteractive(true);

    // 1) 배경
    const QString imgPath = QCoreApplication::applicationDirPath()
                          + "/images/capture_image/capture_image.jpg";
    m_capturePath = imgPath;   // 캡처 경로를 멤버에 저장
    QPixmap captured(imgPath);
    if (captured.isNull()) { qDebug() << "배경 이미지 로드 실패:" << imgPath; return; }

    m_bgItem = m_scene->addPixmap(captured);
    m_bgItem->setZValue(0);

    QRectF bgSceneRect = m_bgItem->mapRectToScene(m_bgItem->boundingRect());
    m_scene->setSceneRect(bgSceneRect);

    // 2) 마스크 로드(+투명화)
    const QString maskFile = (m_puzzleType == 8) ? "puzzle_mask_8x8.png"
                                                 : "puzzle_mask_5x5.png";
    const QString maskPath = QCoreApplication::applicationDirPath()
                           + "/images/" + maskFile;

    QImage maskImg(maskPath);
    if (maskImg.isNull()) { qDebug() << "마스크 로드 실패:" << maskPath; return; }

    maskImg = maskImg.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < maskImg.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(maskImg.scanLine(y));
        for (int x = 0; x < maskImg.width(); ++x) {
            const int r = qRed(line[x]), g = qGreen(line[x]), b = qBlue(line[x]);
            line[x] = (r > 200 && g > 200 && b > 200) ? qRgba(255,255,255,0)
                                                      : qRgba(0,0,0,255);
        }
    }

    QPixmap maskPixmap = QPixmap::fromImage(maskImg);
    QPixmap scaledMask = maskPixmap.scaled(captured.size(),
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation);

    // 4) 마스크 아이템(드래그 가능)
    m_maskItem = new DraggablePixmapItem(scaledMask);
    m_scene->addItem(m_maskItem);

    QRectF maskRect = m_maskItem->boundingRect();
    m_maskItem->setPos( (bgSceneRect.width()  - maskRect.width())  / 2.0,
                        (bgSceneRect.height() - maskRect.height()) / 2.0 );

    // 배경 밖 이동 금지
    m_maskItem->setMoveBounds(bgSceneRect);

    // 보기 맞춤(배경 기준)
    view->fitInView(m_bgItem, Qt::KeepAspectRatio);
}


