#include "puzzleboard.h"
#include "puzzlepiece.h"
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QPainter>

PuzzleBoard::PuzzleBoard(int rows, int cols, const QSizeF& boardSize, QWidget* parent)
    : QGraphicsView(parent), m_rows(rows), m_cols(cols), m_boardSize(boardSize) {
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setDragMode(QGraphicsView::NoDrag);

    m_boardRect = QRectF(0, 0, m_boardSize.width(), m_boardSize.height());
    buildBoard();
    buildSlots();
    fitInView(m_boardRect, Qt::KeepAspectRatio);
}

void PuzzleBoard::buildBoard() {
    if (m_boardRectItem) m_scene->removeItem(m_boardRectItem);
    m_boardRectItem = m_scene->addRect(m_boardRect, QPen(Qt::darkGray, 2), QBrush(Qt::NoBrush));
    m_boardRectItem->setZValue(-100); // 배경
}

void PuzzleBoard::buildSlots() {
    m_slotCenters.clear();
    const qreal cw = m_boardRect.width() / m_cols;
    const qreal ch = m_boardRect.height() / m_rows;
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            QPointF center(m_boardRect.left() + (c + 0.5) * cw,
                           m_boardRect.top()  + (r + 0.5) * ch);
            m_slotCenters.push_back(center);
        }
    }
}

void PuzzleBoard::resizeEvent(QResizeEvent* e) {
    QGraphicsView::resizeEvent(e);
    fitInView(m_boardRect, Qt::KeepAspectRatio);
}

void PuzzleBoard::addPiece(const QPixmap& pix, int id, const QPointF& startScenePos) {
    auto* piece = new PuzzlePiece(pix, this);
    piece->setId(id);
    m_scene->addItem(piece);

    QPointF startPos = startScenePos;
    if (!startPos.isNull()) {
        piece->setPos(startPos);
    } else {
        // 보드 좌상단 근처에 임시 배치
        piece->setPos(m_boardRect.topLeft() + QPointF(8, 8));
    }
}
