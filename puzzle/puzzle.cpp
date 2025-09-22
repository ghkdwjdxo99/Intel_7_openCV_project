#include "puzzle.h"
#include "ui_puzzle.h"
#include "puzzleselectdialog.h"
#include <QMessageBox>
#include <QDebug>

puzzle::puzzle(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::puzzle)
{
    ui->setupUi(this);
}

puzzle::~puzzle()
{
    delete ui;
}

void puzzle::on_cameraButton_clicked()
{
    PuzzleSelectDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        int type = dlg.selectedPuzzleType();
        qDebug() << "카메라 버튼 → 선택된 퍼즐 타입:" << type;

        emit switchToWebcam(type);   // 선택값 전달

        // 필요하다면 메시지박스 대신 스택 전환만 하도록 수정
        /*
        if (type == 5) {
            QMessageBox::information(this, "선택 결과", "5x5 퍼즐 선택됨!");
        } else if (type == 8) {
            QMessageBox::information(this, "선택 결과", "8x8 퍼즐 선택됨!");
        }
        */
    }
}


void puzzle::on_imageButton_clicked()
{
    PuzzleSelectDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        int type = dlg.selectedPuzzleType();
        qDebug() << "이미지 버튼 → 선택된 퍼즐 타입:" << type;

        if (type == 5) {
            QMessageBox::information(this, "선택 결과", "5x5 퍼즐 선택됨!");
        } else if (type == 8) {
            QMessageBox::information(this, "선택 결과", "8x8 퍼즐 선택됨!");
        }
    }

}

