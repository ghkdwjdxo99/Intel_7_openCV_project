#ifndef PUZZLE_H
#define PUZZLE_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class puzzle; }
QT_END_NAMESPACE

class puzzle : public QWidget
{
    Q_OBJECT

public:
    puzzle(QWidget *parent = nullptr);
    ~puzzle();

signals:
    void switchToWebcam();   // 버튼 눌렀을 때 시그널 발생

private slots:
    void on_cameraButton_clicked();
    void on_imageButton_clicked();

private:
    Ui::puzzle *ui;
};
#endif // PUZZLE_H
