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
    void imageImportRequested(int type);    // [ADDED] 이미지 불러오기 플로우 시작 (선택한 퍼즐 타입 전달)


private slots:
    void on_cameraButton_clicked();
    void on_imageButton_clicked();

private:
    Ui::puzzle *ui;
    void clearFolder(const QString &folderPath);
};
#endif // PUZZLE_H
