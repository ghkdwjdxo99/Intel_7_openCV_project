#ifndef PUZZLESELECTDIALOG_H
#define PUZZLESELECTDIALOG_H

#include <QDialog>

namespace Ui {
class PuzzleSelectDialog;
}

class PuzzleSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PuzzleSelectDialog(QWidget *parent = nullptr);
    ~PuzzleSelectDialog();

    int selectedPuzzleType() const;  // 5 또는 8 반환

private slots:
    void on_pushButton_clicked();

private:
    Ui::PuzzleSelectDialog *ui;
    int puzzleType = 0;     // 0=미선택, 5=5x5, 8=8x8
};

#endif // PUZZLESELECTDIALOG_H
