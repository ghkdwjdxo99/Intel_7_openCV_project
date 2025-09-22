#include "fittinggraphicsview.h"
#include <QGraphicsScene>

FittingGraphicsView::FittingGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setAlignment(Qt::AlignCenter);
}

void FittingGraphicsView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);

    if (scene() && !scene()->items().isEmpty()) {
        fitInView(scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
    }
}
