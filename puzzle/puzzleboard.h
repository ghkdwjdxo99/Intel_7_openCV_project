#pragma once
#include <QGraphicsView>
#include <QVector>

class PuzzlePiece;

class PuzzleBoard : public QGraphicsView {
    Q_OBJECT
public:
    explicit PuzzleBoard(int rows, int cols, const QSizeF& boardSize, QWidget* parent = nullptr);
    QRectF boardRect() const { return m_boardRect; }
    const QVector<QPointF>& slotCenters() const { return m_slotCenters; }
    void addPiece(const QPixmap& pix, int id, const QPointF& startScenePos = QPointF());

protected:
    void resizeEvent(QResizeEvent* e) override;

private:
    QGraphicsScene* m_scene = nullptr;
    QGraphicsRectItem* m_boardRectItem = nullptr;
    QRectF m_boardRect;
    int m_rows = 0, m_cols = 0;
    QSizeF m_boardSize;
    QVector<QPointF> m_slotCenters; // 각 슬롯의 중심 좌표(씬 기준)

    void buildBoard();
    void buildSlots();
};
