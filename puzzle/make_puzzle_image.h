#ifndef MAKEPUZZLEIMAGE_H
#define MAKEPUZZLEIMAGE_H

#include <QWidget>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class make_puzzle_image; }
QT_END_NAMESPACE

class QGraphicsScene; // forward declaration

// -------------------------------
// 마스크 드래그용 클래스
// -------------------------------
class DraggablePixmapItem : public QGraphicsPixmapItem {
public:
    DraggablePixmapItem(const QPixmap &pixmap, QGraphicsItem *parent = nullptr)
        : QGraphicsPixmapItem(pixmap, parent)
    {
        setFlag(QGraphicsItem::ItemIsMovable, true);
        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setCursor(Qt::OpenHandCursor);
        setZValue(10);
    }

    void setMoveBounds(const QRectF &bounds) { m_bounds = bounds; }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override {
        setCursor(Qt::ClosedHandCursor);
        QGraphicsPixmapItem::mousePressEvent(event);
    }
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override {
        setCursor(Qt::OpenHandCursor);
        QGraphicsPixmapItem::mouseReleaseEvent(event);
    }
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override {
        QGraphicsPixmapItem::mouseMoveEvent(event);

        if (!scene() || !m_bounds.isValid()) return;

        QRectF itemRect = sceneBoundingRect();
        QPointF p = pos();

        if (itemRect.left()   < m_bounds.left())
            p.setX(p.x() + (m_bounds.left() - itemRect.left()));
        if (itemRect.right()  > m_bounds.right())
            p.setX(p.x() - (itemRect.right() - m_bounds.right()));
        if (itemRect.top()    < m_bounds.top())
            p.setY(p.y() + (m_bounds.top() - itemRect.top()));
        if (itemRect.bottom() > m_bounds.bottom())
            p.setY(p.y() - (itemRect.bottom() - m_bounds.bottom()));

        setPos(p);
    }

private:
    QRectF m_bounds;
};

// -------------------------------
// makePuzzleImage
// -------------------------------
class makePuzzleImage : public QWidget
{
    Q_OBJECT
public:
    makePuzzleImage(QWidget *parent = nullptr);
    ~makePuzzleImage();

    void setPuzzleType(int type) { m_puzzleType = (type == 8 ? 8 : 5); }

signals:
    void showPlayPage();
    void piecesReady();   // ← 새로 추가: 조각 저장 완료 신호

public slots:
    void loadCapturedImage();

private slots:
    void on_make_puzzle_btn_clicked();

private:
    Ui::make_puzzle_image *make_puzzle_image_ui;
    int m_puzzleType {5};

    QGraphicsScene*        m_scene   {nullptr};
    QGraphicsPixmapItem*   m_bgItem  {nullptr};
    DraggablePixmapItem*   m_maskItem{nullptr};

    QString m_capturePath;   // 캡처 이미지 경로
};

#endif // MAKEPUZZLEIMAGE_H
