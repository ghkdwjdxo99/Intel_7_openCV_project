#ifndef PUZZLE_SELECT_QMSGBOX_H
#define PUZZLE_SELECT_QMSGBOX_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class PuzzleSelectQMsgBox; }
QT_END_NAMESPACE

class PuzzleSelectQMsgBox : public QWidget
{
    Q_OBJECT

public:
    explicit PuzzleSelectQMsgBox(QWidget *parent = nullptr);
    ~PuzzleSelectQMsgBox();

private slots:
    void on_cameraButton_clicked();
    void on_imageButton_clicked();

private:
    Ui::PuzzleSelectQMsgBox *ui;
};

#endif // PUZZLE_SELECT_QMSGBOX_H
