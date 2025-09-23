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
    void switchToWebcam(int puzzleType);    // 5 or 8 send

private slots:
    void on_cameraButton_clicked();
    void on_imageButton_clicked();

private:
    Ui::puzzle *ui;
    void clearFolder(const QString &folderPath);
};
#endif // PUZZLE_H
