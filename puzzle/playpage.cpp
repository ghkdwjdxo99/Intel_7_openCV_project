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
#include <QSet>
#include <QFont>
#include <QBrush>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QAbstractAnimation>

// -------------------------
// 파일명에서 숫자(id) 추출
// -------------------------
static int parseIdFromFileName(const QString& name) {
    QRegularExpression re("(\\d+)");
    auto m = re.match(name);
    return m.hasMatch() ? m.captured(1).toInt() : -1;
}

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
    // setPuzzleBoard(5);
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
    ui->HintBT->setText(QString("힌트 (%1회)").arg(hintCount));

    // 방금 저장된 조각 수로 퍼즐 타입 자동 판단
    const QString pieceDir = QCoreApplication::applicationDirPath() + "/images/piece_image";
    QDir d(pieceDir);
    d.setNameFilters({"*.png","*.jpg","*.jpeg","*.bmp"});
    d.setFilter(QDir::Files);
    const int count = d.entryList().size();

    if (count == 25)      setPuzzleBoard(5);
    else if (count == 64) setPuzzleBoard(8);
    else if (count > 0)   setPuzzleBoard(5);
    else                  qDebug() << "[PlayPage] no pieces found yet; waiting.";
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

    // ===== 씬 초기화 (이중 삭제 방지!) =====
    mPieces.clear();   // 포인터 목록만 비움 (실제 삭제는 clear()가 함)
    mScene->clear();   // 씬 내 아이템을 Qt가 delete
    // ===================================

    // 1) 마스크 스케일 & 보드 추가
    QSize tentativeSize(mCols * mCellSize.width(), mRows * mCellSize.height());
    QPixmap scaled = QPixmap::fromImage(maskImg).scaled(
        tentativeSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    auto *board = mScene->addPixmap(scaled);
    board->setZValue(-1);
    board->setPos(mTopLeft);

    // 실제 픽셀 크기에서 셀 크기 계산
    QSize actualPixmapSize = scaled.size();
    QSize actualCellSize(actualPixmapSize.width()  / mCols,
                         actualPixmapSize.height() / mRows);
    mCellSize = actualCellSize;

    QRectF frame(mTopLeft, QSizeF(mCols * mCellSize.width(),
                                  mRows * mCellSize.height()));

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

    // ▼ 슬롯 인덱스(파란색) 시각화: 조각 id와 대조용
    for (int i = 0; i < mSlotCenters.size(); ++i) {
        auto *txt = mScene->addSimpleText(QString::number(i), QFont("Noto Sans KR", 10));
        txt->setPos(mSlotCenters[i] - QPointF(6, 10)); // 약간 위로
        txt->setZValue(1000);
        txt->setBrush(QBrush(Qt::blue));
    }

    // 조각 로드
    QString piecesDir = QCoreApplication::applicationDirPath() + "/images/piece_image";
    loadPiecesFromDir(piecesDir);
}

// 디렉터리에서 조각 로드 후 오른쪽 영역에 랜덤 배치
void PlayPage::loadPiecesFromDir(const QString &dirPath)
{
    // ⚠️ 더 이상 기존 아이템을 remove/delete 하지 않음 (씬 clear 시 이미 삭제됨)
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

    // 결측/중복 간단 점검
    {
        QSet<int> ids;
        for (const auto& fn : files) {
            int id = parseIdFromFileName(fn);
            if (id >= 0) ids.insert(id);
        }
        for (int i = 0; i < need; ++i) {
            if (!ids.contains(i))
                qWarning() << "[Check] missing piece id:" << i;
        }
    }

    auto *rng = QRandomGenerator::global();
    for (int i = 0; i < files.size(); ++i) {
        const QString fn   = files[i];
        const QString path = dir.absoluteFilePath(fn);

        QPixmap px(path);
        if (px.isNull()) { qWarning() << "조각 로드 실패:" << path; continue; }

        // [SIZE] 퍼즐 조각 확대 계수 (필요에 따라 1.10~1.50 등으로 조절)
        const qreal factor = 1.35; // 우선 셀과 동일 크기
        const QSize targetSize(
            qRound(mCellSize.width()  * factor),
            qRound(mCellSize.height() * factor)
        );
        QPixmap scaled = px.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        // 무작위 배치 위치 계산
        qreal minX=mRandomArea.left(), maxX=mRandomArea.right()-scaled.width();
        qreal minY=mRandomArea.top(),  maxY=mRandomArea.bottom()-scaled.height();
        if (maxX<minX) maxX=minX;
        if (maxY<minY) maxY=minY;

        qreal rx = minX+(maxX-minX)*rng->generateDouble();
        qreal ry = minY+(maxY-minY)*rng->generateDouble();

        // 조각 아이템 생성
        auto *item = mScene->addPixmap(scaled);
        item->setPos(rx, ry);
        item->setFlag(QGraphicsItem::ItemIsMovable,true);
        item->setFlag(QGraphicsItem::ItemIsSelectable,true);
        item->setZValue(10);

        // 파일명에서 정답 슬롯 id 파싱
        int parsedId = parseIdFromFileName(fn);
        item->setData(0, parsedId);   // ★ 정답 슬롯 id
        item->setData(1, -1);         // 현재 슬롯 기록

        // --- 앵커 파싱: _ax{X}_ay{Y}
        QRegularExpression re_ax("_ax(-?\\d+)");
        QRegularExpression re_ay("_ay(-?\\d+)\\.");
        int ax = -1, ay = -1;
        auto mx = re_ax.match(fn);
        auto my = re_ay.match(fn);
        if (mx.hasMatch()) ax = mx.captured(1).toInt();
        if (my.hasMatch()) ay = my.captured(1).toInt();

        // 스케일 보정: px → scaled 로 크기가 변했으므로 앵커도 동일 배율 적용
        const qreal sx = scaled.width()  / static_cast<qreal>(px.width());
        const qreal sy = scaled.height() / static_cast<qreal>(px.height());
        QPointF anchorScaled(
            (ax >= 0 ? ax * sx : scaled.width()  / 2.0),
            (ay >= 0 ? ay * sy : scaled.height() / 2.0)
        );

        // data(2)에 로컬좌표계 앵커 저장
        item->setData(2, anchorScaled);

        // 디버그 로그
        qDebug().nospace()
          << "[PieceLoad] file=" << fn
          << " parsedId=" << parsedId
          << " scaled=" << scaled.width() << "x" << scaled.height()
          << " cell=" << mCellSize.width() << "x" << mCellSize.height()
          << " anchor=(" << anchorScaled.x() << "," << anchorScaled.y() << ")";

        // 조각 id 시각화(빨간색): 슬롯 번호(파랑)와 눈으로 대조 가능
        auto *tag = mScene->addSimpleText(QString::number(parsedId), QFont("Noto Sans KR", 9));
        tag->setBrush(Qt::red);
        tag->setZValue(item->zValue()+1);
        tag->setParentItem(item);                 // 아이템에 붙여서 함께 이동
        tag->setPos(QPointF(3,3));

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
    QVariant v = piece->data(2);
    QPointF anchorLocal = v.isValid() ? v.toPointF()
                                      : piece->boundingRect().center(); // fallback
    piece->setPos(targetCenter - anchorLocal);
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
        tolerance = 0.40 * qMin(mCellSize.width(), mCellSize.height()); // 살짝 완화

    const int id = piece->data(0).toInt();
    if (id < 0 || id >= mSlotCenters.size()) {
        qWarning() << "[Snap] invalid id" << id;
        return;
    }

    if (id < mOccupant.size() && mOccupant[id] && mOccupant[id] != piece)
        return;

    const QPointF targetCenter = mSlotCenters[id];

    // 로컬 앵커 → 씬 좌표로 변환
    QVariant v = piece->data(2);
    QPointF anchorLocal = v.isValid() ? v.toPointF()
                                      : piece->boundingRect().center(); // fallback
    QPointF anchorScene = piece->mapToScene(anchorLocal);

    // 앵커 기준 거리
    const qreal dist = QLineF(anchorScene, targetCenter).length();

    qDebug().nospace()
      << "[SnapTry] id=" << id
      << " dist=" << dist
      << " tol=" << tolerance
      << " anchorScene=(" << anchorScene.x() << "," << anchorScene.y() << ")"
      << " target=(" << targetCenter.x() << "," << targetCenter.y() << ")"
      << " pieceSize=" << piece->boundingRect().width() << "x" << piece->boundingRect().height() << ")";

    if (dist <= tolerance) {
        placePieceAtSlot(piece, id);
        qDebug() << "[SnapOK] id=" << id;

        // ─────────────────────────────
        // ✅ 모든 조각이 제자리에 있으면 즉시 최종 이미지로 교체
        //    (중복 실행 방지를 위해 동적 프로퍼티 'finished' 사용)
        bool complete = true;
        for (int i = 0; i < mOccupant.size(); ++i) {
            if (!mOccupant[i] || mOccupant[i]->data(0).toInt() != i) { complete = false; break; }
        }
        if (complete && !this->property("finished").toBool()) {
            this->setProperty("finished", true);

            // 더 이상 스냅/이벤트 안 받도록 약간 정리
            if (timer) timer->stop();
            if (mScene) mScene->removeEventFilter(this);

            // 장면 내용 정리 후 최종 이미지 표시
            mScene->clear();

            const QString finalPath =
                QCoreApplication::applicationDirPath() + "/images/capture_image/puzzle_image.png";
            QPixmap finalPx(finalPath);

            if (!finalPx.isNull()) {
                const QSize boardSize(mCols * mCellSize.width(),
                                      mRows * mCellSize.height());
                QPixmap scaled = finalPx.scaled(boardSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                auto *pix = mScene->addPixmap(scaled);
                pix->setZValue(0);
                pix->setPos(mTopLeft);
            } else {
                qWarning() << "[Finalize] final image load failed:" << finalPath;
            }

            // 필요하면 성공 다이얼로그/시그널
             emit puzzleFinished(elapsedSeconds, true);
        }
        // ─────────────────────────────
    } else {
        qDebug() << "[SnapNG] id=" << id;
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
