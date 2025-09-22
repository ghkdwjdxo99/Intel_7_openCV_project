#include "puzzleselectdialog.h"
#include "ui_puzzleselectdialog.h"
#include "successdialog.h"
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
#if 1
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
#endif
#if 0
    if (ui->check5x5->isChecked()) {
        puzzleType = 5;

        // ✅ 테스트: 선택 시 SuccessDialog 띄우기
        SuccessDialog dlg(this);
        dlg.exec();

        accept();   // 원래 다이얼로그 닫기
    }
    else if (ui->check8x8->isChecked()) {
        puzzleType = 8;

        // ✅ 테스트: 선택 시 SuccessDialog 띄우기
        SuccessDialog dlg(this);
        dlg.exec();

        accept();

    }
    else {
        QMessageBox::warning(this, "선택 오류", "퍼즐 종류를 선택하세요!");
    }
#endif
}

