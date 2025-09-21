#ifndef SELECTPUZZLE_H
#define SELECTPUZZLE_H

#include <QWidget>

namespace Ui {
class SelectPuzzle;
}

class SelectPuzzle : public QWidget
{
    Q_OBJECT

public:
    explicit SelectPuzzle(QWidget *parent = nullptr);
    ~SelectPuzzle();

private:
    Ui::SelectPuzzle *ui;
};

#endif // SELECTPUZZLE_H
