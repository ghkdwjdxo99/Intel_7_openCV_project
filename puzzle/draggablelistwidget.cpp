#include "draggablelistwidget.h"
#include <QDrag>
#include <QMimeData>
#include <QBuffer>
#include <QPixmap>
#include <QListWidgetItem>

DraggableListWidget::DraggableListWidget(QWidget *parent)
    : QListWidget(parent)
{
    setViewMode(QListView::IconMode);
    setIconSize(QSize(100,100));
    setResizeMode(Adjust);
    setDragEnabled(true);
    setSelectionMode(SingleSelection);
}

void DraggableListWidget::startDrag(Qt::DropActions /*supportedActions*/)
{
    QListWidgetItem *it = currentItem();
    if (!it) return;

    QPixmap pix = it->icon().pixmap(iconSize());
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    pix.save(&buffer, "PNG");

    QMimeData *mime = new QMimeData;
    mime->setData("application/x-pixmap", ba);

    QVariant v = it->data(Qt::UserRole);
    if (v.isValid()) {
        mime->setData("application/x-piece-index", QByteArray::number(v.toInt()));
    }

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mime);
    drag->setPixmap(pix);
    drag->exec(Qt::CopyAction);
}
