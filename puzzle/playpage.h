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
    void setPuzzleBoard(int type);  // 퍼즐 보드 설정 함수

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void on_StopBT_clicked();
    void updateTime();

    void on_SolutionBT_clicked();

    void on_HintBT_clicked();

signals:
    void showPuzzle();
    void puzzleFinished(int elapsedSeconds, bool success);

private:
    Ui::PlayPage *ui;
    QTimer *timer;
    int elapsedSeconds;
    int hintCount;
};

#endif // PLAYPAGE_H
