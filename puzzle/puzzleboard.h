#pragma once
#include <QGraphicsView>
#include <QVector>

class PuzzleBoard : public QGraphicsView {
    Q_OBJECT
public:
    explicit PuzzleBoard(int rows, int cols, const QSizeF& boardSize, QWidget* parent = nullptr);
    explicit PuzzleBoard(QWidget* parent=nullptr);
    void configure(int rows, int cols, const QSizeF& boardSize);
    QRectF boardRect() const { return m_boardRect; }
    const QVector<QPointF>& slotCenters() const { return m_slotCenters; }

protected:
    void resizeEvent(QResizeEvent* e) override;

private:
    QGraphicsScene* m_scene = nullptr;
    QGraphicsRectItem* m_boardRectItem = nullptr;
    QList<QGraphicsItem*> m_gridLines;
    QRectF m_boardRect;
    int m_rows = 0, m_cols = 0;
    QSizeF m_boardSize;
    QVector<QPointF> m_slotCenters;

    void buildBoard();
    void buildSlotsAndGrid();
};
