#include "make_puzzle_image.h"
#include "ui_make_puzzle_image.h"
#include "puzzle_piece_save.h"
#include <QGraphicsScene>
#include <QDir>
#include <QPixmap>
#include <QDebug>
#include <QCoreApplication>

#include <opencv2/opencv.hpp>
#include <filesystem>
#include <vector>
#include <algorithm>

// ------------------------------------------------------------
// (헬퍼) 조각의 알파 채널로 가장자리 접촉(돌기) 여부 판정
//  - left/right/top/bottom 중 어디가 이미지 바깥 끝에 닿아있는지 검사
//  - 양옆(L&R) 또는 상하(T&B) 모두 true면 "양쪽/상하 돌기"로 보며 더 크게 스케일
// ------------------------------------------------------------
static inline void detectEdgeTouches(const cv::Mat& bgra,
                                     bool& left, bool& right,
                                     bool& top,  bool& bottom)
{
    left = right = top = bottom = false;
    if (bgra.empty() || bgra.channels() != 4) return;

    std::vector<cv::Mat> ch; cv::split(bgra, ch); // ch[3] = alpha
    const cv::Mat& A = ch[3];

    const int w = A.cols, h = A.rows;
    const int mw = std::max(1, w / 25);  // 대략 4% 폭
    const int mh = std::max(1, h / 25);  // 대략 4% 높이

    if (cv::countNonZero(A(cv::Rect(0,        0, mw, h))) > 0) left   = true;
    if (cv::countNonZero(A(cv::Rect(w - mw,   0, mw, h))) > 0) right  = true;
    if (cv::countNonZero(A(cv::Rect(0,        0, w,  mh))) > 0) top    = true;
    if (cv::countNonZero(A(cv::Rect(0,  h - mh, w,  mh))) > 0) bottom = true;
}

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
    QRectF srcRect = m_maskItem->sceneBoundingRect(); // 씬 좌표
    QSize  outSize = srcRect.size().toSize();
    if (outSize.isEmpty()) {
        qDebug() << "저장 실패: 마스크 크기 0";
        return;
    }

    QImage out(outSize, QImage::Format_ARGB32_Premultiplied);
    out.fill(Qt::transparent);

    // 3) 장면을 해당 영역으로 렌더(= 배경+마스크 합쳐서 잘라내기)
    {
        QPainter p(&out);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setRenderHint(QPainter::SmoothPixmapTransform, true);
        m_scene->render(&p, QRectF(QPointF(0,0), outSize), srcRect);
    }

    // 4) 합성 이미지 저장
    if (out.save(outPath)) {
        qDebug() << "퍼즐 이미지 저장 성공:" << outPath;
    } else {
        qDebug() << "퍼즐 이미지 저장 실패:" << outPath;
    }

    // =========================
    // ⭐ OpenCV 기반 퍼즐 분할
    // =========================
    // 캡처된 '원본' 이미지 (배경 전체)
    cv::Mat cvCapture = cv::imread(m_capturePath.toStdString());
    if (cvCapture.empty()) {
        qWarning() << "캡처 이미지 OpenCV 로드 실패:" << m_capturePath;
        return;
    }

    // 드래그된 마스크 위치(씬 좌표 → 픽셀 ROI)
    cv::Rect roi(
        static_cast<int>(std::round(srcRect.x())),
        static_cast<int>(std::round(srcRect.y())),
        static_cast<int>(std::round(srcRect.width())),
        static_cast<int>(std::round(srcRect.height()))
    );

    // ROI 범위 보정
    roi.x = std::max(0, roi.x);
    roi.y = std::max(0, roi.y);
    roi.width  = std::min(roi.width,  cvCapture.cols - roi.x);
    roi.height = std::min(roi.height, cvCapture.rows - roi.y);
    if (roi.width <= 0 || roi.height <= 0) {
        qWarning() << "ROI 유효 크기 없음!";
        return;
    }

    cv::Mat roiImg = cvCapture(roi).clone();

    // 퍼즐 마스크 로드 (앱 경로 기준)
    const std::string appBase = QCoreApplication::applicationDirPath().toStdString();
    const std::string maskPath =
        appBase + ((m_puzzleType == 8) ? "/images/puzzle_mask_8x8.png"
                                       : "/images/puzzle_mask_5x5.png");

    cv::Mat mask = cv::imread(maskPath, cv::IMREAD_GRAYSCALE);
    if (mask.empty()) {
        qWarning() << "퍼즐 마스크 OpenCV 로드 실패:" << QString::fromStdString(maskPath);
        return;
    }

    // ROI 크기에 맞게 마스크 리사이즈
    cv::Mat maskResized;
    cv::resize(mask, maskResized, roiImg.size());

    // 조각 생성 (P.pos는 roiImg 기준 좌상단 좌표)
    std::vector<PuzzlePiece> pieces = puzzle_piece_save::makePuzzlePieces(roiImg, maskResized);

    // =========================
    // ⭐ r*cols + c 규칙으로 저장
    // =========================
    // 저장 폴더 비우고(파일만 삭제) 다시 생성
    const QString pieceDirQt = QCoreApplication::applicationDirPath() + "/images/piece_image";
    {
        QDir pd(pieceDirQt);
        if (!pd.exists()) QDir().mkpath(pieceDirQt);
        pd.setFilter(QDir::Files);
        const QStringList olds = pd.entryList();
        for (const QString& f : olds) pd.remove(f);
    }
    const std::string pieceDir = pieceDirQt.toStdString();

    const int rows = (m_puzzleType == 8) ? 8 : 5;
    const int cols = (m_puzzleType == 8) ? 8 : 5;
    const int W = roiImg.cols;
    const int H = roiImg.rows;
    const double cellW = static_cast<double>(W) / cols;
    const double cellH = static_cast<double>(H) / rows;

    // id 사용 여부
    std::vector<bool> used(rows * cols, false);

    // 가까운 빈 칸 탐색 (중복/경계 보정용)
    auto findNearestFreeId = [&](int r, int c) -> int {
        for (int rad = 0; rad <= std::max(rows, cols); ++rad) {
            for (int dr = -rad; dr <= rad; ++dr) {
                for (int dc = -rad; dc <= rad; ++dc) {
                    if (std::abs(dr) != rad && std::abs(dc) != rad) continue; // 테두리만
                    int rr = r + dr, cc = c + dc;
                    if (rr < 0 || rr >= rows || cc < 0 || cc >= cols) continue;
                    int cand = rr * cols + cc;
                    if (!used[cand]) return cand;
                }
            }
        }
        // 최후: 선형 검색
        for (int i = 0; i < rows * cols; ++i) if (!used[i]) return i;
        return -1;
    };

    // =========================
    // ⭐ r*cols + c 규칙 + 앵커 저장 + "조각별 배율(fx)" 태깅
    // =========================
    int savedCount = 0;
    for (size_t i = 0; i < pieces.size(); ++i) {
        const auto& P = pieces[i];
        if (P.img.empty() || P.mask.empty()) continue;

        // 1) 어느 셀인가 (r,c)
        const cv::Moments mu = cv::moments(P.mask, true);
        if (mu.m00 <= 1e-6) continue;
        const double cxLocal = mu.m10 / mu.m00;   // 조각 로컬
        const double cyLocal = mu.m01 / mu.m00;

        const double cx = P.pos.x + cxLocal;      // ROI 절대
        const double cy = P.pos.y + cyLocal;

        int c = static_cast<int>(cx / cellW);
        int r = static_cast<int>(cy / cellH);
        c = std::clamp(c, 0, cols - 1);
        r = std::clamp(r, 0, rows - 1);

        int id = r * cols + c;
        if (id < 0 || id >= rows*cols || used[id]) {
            const int fb = findNearestFreeId(r, c);
            if (fb >= 0) id = fb;
        }
        if (id < 0 || id >= rows*cols) {
            qWarning("skip piece: cannot assign id (r=%d c=%d)", r, c);
            continue;
        }
        used[id] = true;

        // 2) "셀 중심" (원본 ROI 좌표)
        const double cellCenterX = (c + 0.5) * cellW;
        const double cellCenterY = (r + 0.5) * cellH;

        // 3) 앵커(셀 중심을 조각 로컬로 투영): anchor = cellCenter - pieceTopLeft
        //    pieceTopLeft = P.pos (ROI 내 좌상단)
        const double anchorX = cellCenterX - P.pos.x;
        const double anchorY = cellCenterY - P.pos.y;
        const int ax = static_cast<int>(std::round(anchorX));
        const int ay = static_cast<int>(std::round(anchorY));

        // 4) 돌기 분류 → 조각별 배율 결정
        bool L=false, R=false, T=false, B=false;
        detectEdgeTouches(P.img, L, R, T, B);
        double fx = 1.00;                       // 기본(한쪽만 돌기 등)
        if ((L && R) || (T && B)) fx = 1.35;    // 양옆 또는 상하 돌기 → 더 크게 (튜닝 가능)

        const int fxInt = static_cast<int>(std::round(fx * 100.0)); // 125 등

        // 5) 파일명: id + anchor + factor
        const std::string outPath = pieceDir + "/piece_" + std::to_string(id)
                                  + "_ax" + std::to_string(ax)
                                  + "_ay" + std::to_string(ay)
                                  + "_fx" + std::to_string(fxInt)
                                  + ".png";

        if (!cv::imwrite(outPath, P.img)) {
            qWarning("imwrite failed: %s", outPath.c_str());
        } else {
            ++savedCount;
            qDebug("[PieceSave] id=%d r=%d c=%d  anchor=(%d,%d)  fx=%d  path=%s",
                   id, r, c, ax, ay, fxInt, outPath.c_str());
        }
    }

    qDebug("[PieceSave] total saved: %d (rows=%d cols=%d, cell=%dx%d)",
           savedCount, rows, cols, (int)std::round(cellW), (int)std::round(cellH));

    // PlayPage에서 연결되어 있을 piecesReady() 알림
    emit piecesReady();

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
    m_capturePath = imgPath;   // 캡처 경로 저장
    QPixmap captured(imgPath);
    if (captured.isNull()) { qDebug() << "배경 이미지 로드 실패:" << imgPath; return; }

    m_bgItem = m_scene->addPixmap(captured);
    m_bgItem->setZValue(0);

    QRectF bgSceneRect = m_bgItem->mapRectToScene(m_bgItem->boundingRect());
    m_scene->setSceneRect(bgSceneRect);

    // 2) 마스크 로드(+흰색 투명화, 검정 불투명)
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
            line[x] = (r > 200 && g > 200 && b > 200) ? qRgba(255,255,255,0)  // 흰색→투명
                                                      : qRgba(0,0,0,255);     // 검정→불투명
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
