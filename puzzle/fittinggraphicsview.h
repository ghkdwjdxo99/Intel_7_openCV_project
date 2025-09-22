#ifndef FITTINGGRAPHICSVIEW_H
#define FITTINGGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QResizeEvent>

class FittingGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit FittingGraphicsView(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // FITTINGGRAPHICSVIEW_H
