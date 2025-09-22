#pragma once
#include <QGraphicsPixmapItem>

class PuzzleBoard;

class PuzzlePiece : public QGraphicsPixmapItem {
public:
    PuzzlePiece(const QPixmap& pix, PuzzleBoard* board, QGraphicsItem* parent = nullptr);

    int id() const { return m_id; }
    void setId(int v) { m_id = v; }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* e) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* e) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override;

private:
    PuzzleBoard* m_board = nullptr;
    QPointF m_pressOffset; // press 시점: itemPos - scenePos
    int m_id = -1;

    QPointF clampTopLeftToBoard(const QPointF& wanted) const;
    QPointF nearestSlotTopLeft() const;
};
