#include "puzzleselectdialog.h"
#include "ui_puzzleselectdialog.h"
#include <QMessageBox>

PuzzleSelectDialog::PuzzleSelectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PuzzleSelectDialog)
{
    ui->setupUi(this);

    puzzleType = -1;  // 초기화 (선택 안 됐을 때 구분용)
}

PuzzleSelectDialog::~PuzzleSelectDialog()
{
    delete ui;
}

int PuzzleSelectDialog::selectedPuzzleType() const {
    return puzzleType;
}


void PuzzleSelectDialog::on_pushButton_clicked()
{
    if (ui->check5x5->isChecked()) {
        puzzleType = 5;
        accept();
    }
    else if (ui->check8x8->isChecked()) {
        puzzleType = 8;
        accept();
    }
    else {
        QMessageBox::warning(this, "선택 오류", "퍼즐 종류를 선택하세요!");
    }
}

