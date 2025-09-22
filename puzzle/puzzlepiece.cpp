#include "puzzlepiece.h"
#include "puzzleboard.h"
#include <QGraphicsSceneMouseEvent>
#include <QtMath>
#include <limits>

PuzzlePiece::PuzzlePiece(const QPixmap& pix, PuzzleBoard* board, QGraphicsItem* parent)
    : QGraphicsPixmapItem(pix, parent), m_board(board) {
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setAcceptedMouseButtons(Qt::LeftButton);
    setCursor(Qt::OpenHandCursor);
    setZValue(1);
}

void PuzzlePiece::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    setFocus();
    setZValue(100);
    setCursor(Qt::ClosedHandCursor);
    m_pressOffset = pos() - e->scenePos();
    QGraphicsPixmapItem::mousePressEvent(e);
}

void PuzzlePiece::mouseMoveEvent(QGraphicsSceneMouseEvent* e) {
    QPointF wanted = e->scenePos() + m_pressOffset;     // 아이템 좌상단 기준 이동
    setPos(clampTopLeftToBoard(wanted));                // 보드 내부로 클램프
    QGraphicsPixmapItem::mouseMoveEvent(e);
}

void PuzzlePiece::mouseReleaseEvent(QGraphicsSceneMouseEvent* e) {
    setCursor(Qt::OpenHandCursor);
    setZValue(1);
    // 가장 가까운 슬롯으로 스냅 (정답 여부와 무관)
    setPos(nearestSlotTopLeft());
    QGraphicsPixmapItem::mouseReleaseEvent(e);
}

QPointF PuzzlePiece::clampTopLeftToBoard(const QPointF& wanted) const {
    const QRectF br = m_board->boardRect();
    const QSizeF ps(boundingRect().width(), boundingRect().height());
    const QRectF allowed(br.left(), br.top(), br.width() - ps.width(), br.height() - ps.height());
    const qreal x = qBound(allowed.left(), wanted.x(), allowed.right());
    const qreal y = qBound(allowed.top(),  wanted.y(), allowed.bottom());
    return QPointF(x, y);
}

QPointF PuzzlePiece::nearestSlotTopLeft() const {
    const auto& centers = m_board->slotCenters();
    if (centers.isEmpty()) return pos();

    const QPointF pieceCenter = pos() + boundingRect().center();

    int bestIdx = 0;
    qreal bestD2 = std::numeric_limits<qreal>::max();
    for (int i = 0; i < centers.size(); ++i) {
        const qreal dx = pieceCenter.x() - centers[i].x();
        const qreal dy = pieceCenter.y() - centers[i].y();
        const qreal d2 = dx*dx + dy*dy;      // ← 수동으로 제곱거리 계산
        if (d2 < bestD2) { bestD2 = d2; bestIdx = i; }
    }
    return centers[bestIdx] - boundingRect().center();
}
