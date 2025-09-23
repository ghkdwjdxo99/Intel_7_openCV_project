#include "puzzleboard.h"
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QPainter>

PuzzleBoard::PuzzleBoard(int rows, int cols, const QSizeF& boardSize, QWidget* parent)
    : QGraphicsView(parent), m_rows(rows), m_cols(cols), m_boardSize(boardSize) {
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setDragMode(QGraphicsView::NoDrag);

    m_boardRect = QRectF(0, 0, m_boardSize.width(), m_boardSize.height());
    buildBoard();
    buildSlotsAndGrid();
    fitInView(m_boardRect, Qt::KeepAspectRatio);
}

PuzzleBoard::PuzzleBoard(QWidget* parent)
    : QGraphicsView(parent), m_rows(8), m_cols(8), m_boardSize(640,640) {
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setDragMode(QGraphicsView::NoDrag);
    m_boardRect = QRectF(0,0,m_boardSize.width(),m_boardSize.height());
    buildBoard(); buildSlotsAndGrid();
    fitInView(m_boardRect, Qt::KeepAspectRatio);
}

void PuzzleBoard::configure(int rows, int cols, const QSizeF& boardSize) {
    m_rows = rows; m_cols = cols; m_boardSize = boardSize;
    m_boardRect = QRectF(0,0,boardSize.width(),boardSize.height());
    buildBoard(); buildSlotsAndGrid();
    fitInView(m_boardRect, Qt::KeepAspectRatio);
}

void PuzzleBoard::buildBoard() {
    if (m_boardRectItem) {
        m_scene->removeItem(m_boardRectItem);
        m_boardRectItem = nullptr;
    }
    m_boardRectItem = m_scene->addRect(m_boardRect, QPen(Qt::darkGray, 2), QBrush(Qt::NoBrush));
    m_boardRectItem->setZValue(-100);
}

void PuzzleBoard::buildSlotsAndGrid() {
    // 기존 그리드 라인 제거
    for (auto* it : m_gridLines) m_scene->removeItem(it);
    m_gridLines.clear();
    m_slotCenters.clear();

    const qreal cw = m_boardRect.width() / m_cols;
    const qreal ch = m_boardRect.height() / m_rows;

    // 슬롯 중심 계산
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            m_slotCenters.push_back({
                m_boardRect.left() + (c + 0.5) * cw,
                m_boardRect.top()  + (r + 0.5) * ch
            });
        }
    }

    // 얇은 그리드 라인(퍼즐 모양 X, 직사각형 분할선만)
    QPen pen(QColor(180,180,180));
    pen.setWidthF(0.5);

    // 세로선
    for (int c = 1; c < m_cols; ++c) {
        auto* ln = m_scene->addLine(
            m_boardRect.left() + c * cw, m_boardRect.top(),
            m_boardRect.left() + c * cw, m_boardRect.bottom(), pen);
        ln->setZValue(-50);
        m_gridLines << ln;
    }
    // 가로선
    for (int r = 1; r < m_rows; ++r) {
        auto* ln = m_scene->addLine(
            m_boardRect.left(), m_boardRect.top() + r * ch,
            m_boardRect.right(), m_boardRect.top() + r * ch, pen);
        ln->setZValue(-50);
        m_gridLines << ln;
    }
}

void PuzzleBoard::resizeEvent(QResizeEvent* e) {
    QGraphicsView::resizeEvent(e);
    fitInView(m_boardRect, Qt::KeepAspectRatio);
}
