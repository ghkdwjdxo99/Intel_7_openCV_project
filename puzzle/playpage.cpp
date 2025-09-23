#include "playpage.h"
#include "ui_playpage.h"
#include "puzzle.h"
#include "successdialog.h"
#include "faildialog.h"
#include "solutiondialog.h"

#include <QStackedWidget>
#include <QMetaObject>
#include <QDebug>
#include <QTimer>
#include <QMessageBox>

#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsPixmapItem>
#include <QRandomGenerator>
#include <QCoreApplication>

PlayPage::PlayPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlayPage),
    timer(new QTimer(this)),
    elapsedSeconds(0)
{
    ui->setupUi(this);

    // 타이머와 슬롯 연결
    connect(timer, &QTimer::timeout, this, &PlayPage::updateTime);

    // 초기 라벨 값 설정
    ui->timerLabel->setText("00:00");

    // 힌트 횟수 초기화
    hintCount = 3;
    ui->HintBT->setText("힌트 (3회)");

    // === GraphicsScene 연결 ===
    mScene = new QGraphicsScene(this);
    ui->PuzzleBoard->setScene(mScene);
    ui->PuzzleBoard->setSceneRect(0, 0, 1280, 720);


    mScene->installEventFilter(this);

    // 기본 퍼즐판 생성 (예: 5x5)
    setPuzzleBoard(5);

    // 테스트용 조각 2개 추가
    auto addOnePiece = [&](const QString &filePath) {
        QPixmap px(filePath);
        if (px.isNull()) {
            qWarning() << "조각 이미지 로드 실패:" << filePath;
            return;
        }

        QPixmap scaled = px.scaled(mCellSize.width(), mCellSize.height(),
                                   Qt::KeepAspectRatioByExpanding,
                                   Qt::SmoothTransformation);

        auto *rng = QRandomGenerator::global();
        const qreal minX = mRandomArea.left();
        const qreal maxX = mRandomArea.right() - scaled.width();
        const qreal minY = mRandomArea.top();
        const qreal maxY = mRandomArea.bottom() - scaled.height();
        const qreal rx = minX + (maxX - minX) * rng->generateDouble();
        const qreal ry = minY + (maxY - minY) * rng->generateDouble();

        auto *item = mScene->addPixmap(scaled);
        item->setPos(rx, ry);
        item->setFlag(QGraphicsItem::ItemIsMovable, true);   // 드래그 가능
        item->setFlag(QGraphicsItem::ItemIsSelectable, true);
        item->setZValue(10);
        mPieces.push_back(item);
    };

    addOnePiece(QCoreApplication::applicationDirPath()+"/images/piece_image/piece_0.png");
    addOnePiece(QCoreApplication::applicationDirPath()+"/images/piece_image/piece_1.png");
}

PlayPage::~PlayPage()
{
    delete ui;
}

void PlayPage::on_StopBT_clicked()
{
    timer->stop();  // 타이머 멈춤
    emit puzzleFinished(elapsedSeconds, false); //포기 시그널 발생
}

void PlayPage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    elapsedSeconds = 0;
    ui->timerLabel->setText("00:00");
    timer->start(1000);  // 1초마다 updateTime 실행

    hintCount = 3;
    ui->HintBT->setText("힌트 (3회)");
}

void PlayPage::updateTime()
{
    elapsedSeconds++;

    int minutes = elapsedSeconds / 60;
    int seconds = elapsedSeconds % 60;

    ui->timerLabel->setText(
        QString("%1:%2")
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'))
    );
}

void PlayPage::on_SolutionBT_clicked()
{
   SolutionDialog dlg(this);
   dlg.setImage("./images/capture_image/puzzle_image.png");  // 이미지 경로 전달
   dlg.exec();
}

void PlayPage::on_HintBT_clicked()
{
    if(hintCount > 0){
        hintCount--;
        ui->HintBT->setText(QString("힌트 (%1회)").arg(hintCount));
        qDebug() << "힌트 사용! 남은 횟수:" << hintCount;
    }
    else{
        QMessageBox::warning(this, "힌트 제한", "더 이상 힌트를 사용할 수 없습니다!!!");
    }
}

// 퍼즐판(격자) 세팅
void PlayPage::setPuzzleBoard(int type)
{
    QString maskPath;
    if (type == 5) {
        mRows = 5; mCols = 5;
        maskPath = QCoreApplication::applicationDirPath() + "/images/puzzle_mask_5x5.png";
    }
    else if (type == 8) {
        mRows = 8; mCols = 8;
        maskPath = QCoreApplication::applicationDirPath() + "/images/puzzle_mask_8x8.png";
    }
    else return;

    QImage maskImg(maskPath);
    if (maskImg.isNull()) {
        qWarning() << "퍼즐 마스크 이미지 로드 실패:" << maskPath;
        return;
    }

    // 필요 시 흰색 → 투명 처리
    maskImg = maskImg.convertToFormat(QImage::Format_ARGB32);
    for (int y=0; y<maskImg.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(maskImg.scanLine(y));
        for (int x=0; x<maskImg.width(); ++x) {
            int r = qRed(line[x]);
            int g = qGreen(line[x]);
            int b = qBlue(line[x]);
            if (r > 200 && g > 200 && b > 200) {
                line[x] = qRgba(255,255,255,0); // 흰색 투명화
            }
        }
    }

    // 🎯 씬 초기화 후 마스크 올리기
    mScene->clear();

    // 퍼즐판 전체 크기 = 행 * 셀 크기
    QSize targetSize(mCols * mCellSize.width(),
                     mRows * mCellSize.height());

    QPixmap scaled = QPixmap::fromImage(maskImg).scaled(
        targetSize,
        Qt::IgnoreAspectRatio,
        Qt::SmoothTransformation
    );

    auto *item = mScene->addPixmap(scaled);
    item->setZValue(-1);
    item->setPos(mTopLeft); // (40,40)에 맞춰서
}






// 거리 판정 후 스냅 함수
void PlayPage::trySnap(QGraphicsPixmapItem *piece, double tolerance)
{
    if (!piece) return;

    // 조각 중심
    QPointF pc = piece->sceneBoundingRect().center();

    // 모든 슬롯 검사
    for (auto *slot : mSlots) {
        QPointF sc = slot->rect().center() + slot->pos();
        qreal dist = QLineF(pc, sc).length();

        if (dist < tolerance) {
            // 스냅 성공 → 슬롯 중심에 고정
            piece->setPos(sc - piece->boundingRect().center());
            piece->setFlag(QGraphicsItem::ItemIsMovable, false); // 잠금
            qDebug() << "스냅 성공!";
            return;
        }
    }
}

// mouseReleaseEvent 대체 — scene 이벤트 잡아서 스냅
bool PlayPage::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == mScene && event->type() == QEvent::GraphicsSceneMouseRelease) {
        auto items = mScene->selectedItems();
        if (!items.isEmpty()) {
            auto *piece = dynamic_cast<QGraphicsPixmapItem*>(items.first());
            if (piece) trySnap(piece, 30.0); // 30px 안이면 스냅
        }
    }
    return QWidget::eventFilter(watched, event);
}
