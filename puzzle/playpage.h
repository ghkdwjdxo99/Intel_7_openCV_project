#ifndef PLAYPAGE_H
#define PLAYPAGE_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class PlayPage;
}

class PlayPage : public QWidget
{
    Q_OBJECT

public:
    explicit PlayPage(QWidget *parent = nullptr);
    ~PlayPage();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void on_StopBT_clicked();
    void updateTime();

signals:
    void showPuzzle();
    void puzzleFinished(int elapsedSeconds, bool success);

private:
    Ui::PlayPage *ui;
    QTimer *timer;
    int elapsedSeconds;
};

#endif // PLAYPAGE_H
