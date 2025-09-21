#ifndef PLAYPAGE_H
#define PLAYPAGE_H

#include <QWidget>

namespace Ui {
class PlayPage;
}

class PlayPage : public QWidget
{
    Q_OBJECT

public:
    explicit PlayPage(QWidget *parent = nullptr);
    ~PlayPage();

private slots:
    void on_StBT_clicked();


signals:
    void showPuzzle();


private:
    Ui::PlayPage *ui;
};

#endif // PLAYPAGE_H
