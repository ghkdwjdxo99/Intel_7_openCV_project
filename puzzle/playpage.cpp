#include "playpage.h"
#include "ui_playpage.h"
#include "puzzle.h"
#include "successdialog.h"
#include "faildialog.h"
#include "solutiondialog.h"
#include "puzzleboard.h"

#include <QStackedWidget>
#include <QMetaObject>
#include <QDebug>
#include <QTimer>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QCoreApplication>
#include <QDir>
#include <QImage>
#include <QGraphicsSceneMouseEvent>
#include <QRegularExpression>
#include <QLineF>
#include <limits>
#include <QDateTime>

PlayPage::PlayPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlayPage),
    timer(new QTimer(this)),
    elapsedSeconds(0)
{
    ui->setupUi(this);

    // 타이머 연결
    connect(timer, &QTimer::timeout, this, &PlayPage::updateTime);
    ui->timerLabel->setText("00:00");

    // 힌트 초기화
    hintCount = 3;
    ui->HintBT->setText("힌트 (3회)");

    // Scene 연결
    mScene = new QGraphicsScene(this);
    ui->PuzzleBoardView->setScene(mScene);
    ui->PuzzleBoardView->setSceneRect(0, 0, 1280, 720);

    // 씬 이벤트 필터(마우스 릴리즈 시 스냅)
    mScene->installEventFilter(this);

    // ⚠️ 여기서 미리 보드를 만들지 않는다.
    // setPuzzleBoard(5);  // 제거!
}

PlayPage::~PlayPage()
{
    delete ui;
}

void PlayPage::on_StopBT_clicked()
{
    timer->stop();
    emit puzzleFinished(elapsedSeconds, false);
}

void PlayPage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    elapsedSeconds = 0;
    ui->timerLabel->setText("00:00");
    timer->start(1000);

    hintCount = 3;
    ui->HintBT->setText("힌트 (3회)");

    // 👉 방금 저장된 조각이 있으면 개수로 5x5(25) / 8x8(64) 자동 판단해서 로드
    const QString pieceDir = QCoreApplication::applicationDirPath() + "/images/piece_image";
    QDir d(pieceDir);
    d.setNameFilters({"*.png","*.jpg","*.jpeg","*.bmp"});
    d.setFilter(QDir::Files);
    const int count = d.entryList().size();

    if (count == 25) {
        setPuzzleBoard(5);
    } else if (count == 64) {
        setPuzzleBoard(8);
    } else if (count > 0) {
        // 애매하면 5x5로 시도 (필요시 조정)
        setPuzzleBoard(5);
    } else {
        // 폴더가 비었으면 아무 것도 하지 않음 (piecesReady()로 불러오는 구성이면 자연스럽게 로드됨)
        qDebug() << "[PlayPage] no pieces found yet; waiting.";
    }
}

void PlayPage::updateTime()
{
    elapsedSeconds++;
    int minutes = elapsedSeconds / 60;
    int seconds = elapsedSeconds % 60;

    ui->timerLabel->setText(
        QString("%1:%2").arg(minutes, 2, 10, QChar('0'))
                        .arg(seconds, 2, 10, QChar('0'))
    );
}

void PlayPage::on_SolutionBT_clicked()
{
   SolutionDialog dlg(this);
   dlg.setImage(QCoreApplication::applicationDirPath() + "/images/capture_image/puzzle_image.png");
   dlg.exec();
}

void PlayPage::on_HintBT_clicked()
{
    if (hintCount > 0) {
        hintCount--;
        ui->HintBT->setText(QString("힌트 (%1회)").arg(hintCount));
        qDebug() << "힌트 사용! 남은 횟수:" << hintCount;
    } else {
        QMessageBox::warning(this, "힌트 제한", "더 이상 힌트를 사용할 수 없습니다!!!");
    }
}

// 퍼즐판 세팅 (마스크 + 슬롯 중심 계산 + 조각 로드)
void PlayPage::setPuzzleBoard(int type)
{
    QString maskPath;
    if (type == 5) { mRows = 5; mCols = 5; maskPath = QCoreApplication::applicationDirPath() + "/images/puzzle_mask_5x5.png"; }
    else if (type == 8) { mRows = 8; mCols = 8; maskPath = QCoreApplication::applicationDirPath() + "/images/puzzle_mask_8x8.png"; }
    else return;

    QImage maskImg(maskPath);
    if (maskImg.isNull()) {
        qWarning() << "퍼즐 마스크 로드 실패:" << maskPath;
        return;
    }

    // 흰색 → 투명
    maskImg = maskImg.convertToFormat(QImage::Format_ARGB32);
    for (int y=0; y<maskImg.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(maskImg.scanLine(y));
        for (int x=0; x<maskImg.width(); ++x) {
            int r=qRed(line[x]), g=qGreen(line[x]), b=qBlue(line[x]);
            if (r>200 && g>200 && b>200) line[x] = qRgba(255,255,255,0);
        }
    }

    // 씬 초기화
    mScene->clear();

    // 1) 원본 마스크를 우선 타겟 사이즈(행*셀, 열*셀) 기준으로 스케일하지만,
    //    실제 스케일 결과의 픽셀 사이즈에서 셀 크기를 재계산하여 정확히 맞춘다.
    QSize tentativeSize(mCols * mCellSize.width(), mRows * mCellSize.height());
    QPixmap scaled = QPixmap::fromImage(maskImg).scaled(
        tentativeSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 보드 아이템 추가
    auto *board = mScene->addPixmap(scaled);
    board->setZValue(-1);
    board->setPos(mTopLeft);

    // --- 핵심: 실제 픽셀 크기에서 정확한 셀 크기 계산 ---
    QSize actualPixmapSize = scaled.size();
    QSize actualCellSize(actualPixmapSize.width()  / mCols,
                         actualPixmapSize.height() / mRows);
    mCellSize = actualCellSize;   // 덮어쓰기: 이후 슬롯/조각에 사용될 값
    QRectF frame(mTopLeft, QSizeF(mCols * mCellSize.width(),
                                  mRows * mCellSize.height()));
    // --- 끝 ---

    // 슬롯 중심 좌표 계산
    mSlotCenters.clear();
    for (int r = 0; r < mRows; ++r) {
        for (int c = 0; c < mCols; ++c) {
            QRectF cell(
                mTopLeft.x() + c * mCellSize.width(),
                mTopLeft.y() + r * mCellSize.height(),
                mCellSize.width(),
                mCellSize.height()
            );
            mSlotCenters.push_back(cell.center());
        }
    }

    // 슬롯 점유 초기화
    mOccupant.clear();
    mOccupant.resize(mRows * mCols);
    for (auto &p : mOccupant) p = nullptr;

    // 무작위 배치 영역
    mRandomArea = QRectF(frame.right()+40, frame.top(),
                         1280-(frame.right()+40)-40, frame.height());

    // 조각 로드
    QString piecesDir = QCoreApplication::applicationDirPath() + "/images/piece_image";
    loadPiecesFromDir(piecesDir);
}

// 디렉터리에서 조각 로드 후 오른쪽 영역에 랜덤 배치
void PlayPage::loadPiecesFromDir(const QString &dirPath)
{
    for (auto *it : mPieces) { mScene->removeItem(it); delete it; }
    mPieces.clear();

    QDir dir(dirPath);
    if (!dir.exists()) {
        qWarning() << "조각 폴더 없음:" << dirPath;
        return;
    }

    dir.setNameFilters({"*.png","*.jpg","*.jpeg","*.bmp"});
    dir.setSorting(QDir::Name); // piece_0, piece_1 ... 순

    QStringList files = dir.entryList();
    int need = mRows*mCols;
    files = files.mid(0, qMin(need, files.size()));

    auto *rng = QRandomGenerator::global();
    for (int i = 0; i < files.size(); ++i) {
        QString fn   = files[i];
        QString path = dir.absoluteFilePath(fn);
        QPixmap px(path);
        if (px.isNull()) { qWarning() << "조각 로드 실패:" << path; continue; }

        QPixmap scaled = px.scaled(mCellSize, Qt::KeepAspectRatioByExpanding,
                                   Qt::SmoothTransformation);

        qreal minX=mRandomArea.left(), maxX=mRandomArea.right()-scaled.width();
        qreal minY=mRandomArea.top(),  maxY=mRandomArea.bottom()-scaled.height();
        if (maxX<minX) maxX=minX;
        if (maxY<minY) maxY=minY;

        qreal rx = minX+(maxX-minX)*rng->generateDouble();
        qreal ry = minY+(maxY-minY)*rng->generateDouble();

        auto *item = mScene->addPixmap(scaled);
        item->setPos(rx, ry);
        item->setFlag(QGraphicsItem::ItemIsMovable,true);
        item->setFlag(QGraphicsItem::ItemIsSelectable,true);
        item->setZValue(10);

        item->setData(0, i);   // 정답 슬롯 id
        item->setData(1, -1);  // 현재 슬롯 기록

        mPieces.push_back(item);
    }
    qDebug() << "조각 로드 완료:" << mPieces.size();
    qDebug().nospace()
        << "[" << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz") << "] "
        << "Loading pieces from " << dirPath << "  filesFound=" << files.size()
        << "  need=" << need;
}

// 가장 가까운 "빈" 슬롯 인덱스 찾기
int PlayPage::nearestFreeSlotIndex(const QPointF& pieceCenter, qreal& outDist) const
{
    outDist = std::numeric_limits<qreal>::max();
    int best = -1;

    for (int i = 0; i < mSlotCenters.size(); ++i) {
        if (i < mOccupant.size() && mOccupant[i]) continue; // 점유된 슬롯 제외
        qreal d = QLineF(pieceCenter, mSlotCenters[i]).length();
        if (d < outDist) { outDist = d; best = i; }
    }
    return best;
}

// 주어진 슬롯에 조각 배치(잠금/점유 표시 포함)
void PlayPage::placePieceAtSlot(QGraphicsPixmapItem* piece, int slotIndex)
{
    if (!piece) return;
    if (slotIndex < 0 || slotIndex >= mSlotCenters.size()) return;

    // 이전 슬롯 점유 해제
    int prev = piece->data(1).toInt();
    if (prev >= 0 && prev < mOccupant.size() && mOccupant[prev] == piece) {
        mOccupant[prev] = nullptr;
    }

    // 새 슬롯으로 스냅
    const QPointF targetCenter = mSlotCenters[slotIndex];
    piece->setPos(targetCenter - piece->boundingRect().center());
    piece->setFlag(QGraphicsItem::ItemIsMovable, false);
    piece->setFlag(QGraphicsItem::ItemIsSelectable, false);
    piece->setZValue(1);

    if (slotIndex < mOccupant.size()) mOccupant[slotIndex] = piece;
    piece->setData(1, slotIndex);
}

// 스냅: 정답 슬롯에만 끌림
void PlayPage::trySnap(QGraphicsPixmapItem *piece, double tolerance)
{
    if (!piece) return;

    if (tolerance <= 0.0)
        tolerance = 0.35 * qMin(mCellSize.width(), mCellSize.height());

    const int id = piece->data(0).toInt();
    if (id < 0 || id >= mSlotCenters.size()) return;

    if (id < mOccupant.size() && mOccupant[id] && mOccupant[id] != piece)
        return;

    const QPointF targetCenter = mSlotCenters[id];
    const QPointF pieceCenter  = piece->sceneBoundingRect().center();
    const qreal   dist         = QLineF(pieceCenter, targetCenter).length();

    if (dist <= tolerance) {
        placePieceAtSlot(piece, id);
        qDebug() << "정답 스냅 성공! id=" << id << "dist=" << dist;
    }
}

// 마우스 릴리즈 시 스냅 시도
bool PlayPage::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == mScene && event->type() == QEvent::GraphicsSceneMouseRelease) {
        auto *e = static_cast<QGraphicsSceneMouseEvent*>(event);
        const QPointF pos = e->scenePos();

        const auto under = mScene->items(pos);
        for (QGraphicsItem *it : under) {
            if (auto *piece = qgraphicsitem_cast<QGraphicsPixmapItem*>(it)) {
                trySnap(piece, 0.0); // 0.0 → 내부에서 비율로 tol 계산
                break;
            }
        }
    }
    return QWidget::eventFilter(watched,event);
}
