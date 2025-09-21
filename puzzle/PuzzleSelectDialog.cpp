#include "PuzzleSelectDialog.h"
#include "ui_PuzzleSelectDialog.h"
#include <QMessageBox>

PuzzleSelectDialog::PuzzleSelectDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PuzzleSelectDialog)
{
    ui->setupUi(this);

    // 퍼즐 마스크 이미지 미리보기
    ui->preview5x5->setPixmap(QPixmap(":/images/puzzle_mask.png").scaled(120,120, Qt::KeepAspectRatio));
    ui->preview8x8->setPixmap(QPixmap(":/images/puzzle_mask2.png").scaled(120,120, Qt::KeepAspectRatio));
}

PuzzleSelectDialog::~PuzzleSelectDialog()
{
    delete ui;
}

int PuzzleSelectDialog::selectedPuzzleType() const {
    return puzzleType;
}

void PuzzleSelectDialog::on_selectButton_clicked()
{
    if (ui->check5x5->isChecked()) {
        puzzleType = 5;
        accept(); // 다이얼로그 닫기
    }
    else if (ui->check8x8->isChecked()) {
        puzzleType = 8;
        accept();
    }
    else {
        QMessageBox::warning(this, "선택 오류", "퍼즐 종류를 선택하세요!");
    }
}
