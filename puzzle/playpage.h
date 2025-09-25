#ifndef PLAYPAGE_H
#define PLAYPAGE_H

#include <QWidget>
#include <QTimer>

#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsPixmapItem>
#include <QVector>
#include <QSize>
#include <QPointF>
#include <QRectF>

namespace Ui {
class PlayPage;
}

class PlayPage : public QWidget
{
    Q_OBJECT

public:
    explicit PlayPage(QWidget *parent = nullptr);
    ~PlayPage();
    void setPuzzleBoard(int type);  // 퍼즐 보드 설정 함수]
    void trySnap(QGraphicsPixmapItem *piece, double tolerance);

protected:
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void on_StopBT_clicked();
    void updateTime();

    void on_SolutionBT_clicked();

    void on_HintBT_clicked();

signals:
    void showPuzzle();
    void puzzleFinished(int elapsedSeconds, bool success);

private:
    Ui::PlayPage *ui;
    QTimer *timer;
    int elapsedSeconds;
    int hintCount;
    void loadPiecesFromDir(const QString &dirPath);

    // 🎯 퍼즐판을 그릴 QGraphicsScene
    QGraphicsScene *mScene = nullptr;

    // 🎯 슬롯(퍼즐 보드 칸) 저장 (나중에 스냅/정답 판정에 씀)
    QVector<QGraphicsRectItem*> mSlots;

    // 🎯 조각 저장 (Step 2에서 단순히 드래그만, Step 3부터 스냅에 활용)
    QVector<QGraphicsPixmapItem*> mPieces;

    // 각 슬롯(칸)의 정답 중심 좌표 [id = r*cols + c]
    QVector<QPointF> mSlotCenters;
    // (선택) 슬롯 점유 상태: 해당 슬롯에 붙은 조각 포인터, 없으면 nullptr
    QVector<QGraphicsPixmapItem*> mOccupant;

    // 근접 빈 슬롯 찾기 / 배치 함수
    int  nearestFreeSlotIndex(const QPointF& pieceCenter, qreal& outDist) const;
    void placePieceAtSlot(QGraphicsPixmapItem* piece, int slotIndex);

    // 🎯 퍼즐판 행/열, 셀 크기, 좌상단 위치 (초기값은 임시로 세팅)
    int mRows = 4;
    int mCols = 6;
    QSize mCellSize = QSize(96, 96);
    QPointF mTopLeft = QPointF(40, 40);

    // 🎯 조각을 무작위로 뿌릴 영역 (퍼즐판 오른쪽 여백)
    QRectF mRandomArea;

};

#endif // PLAYPAGE_H
