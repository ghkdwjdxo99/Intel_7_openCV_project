#include "puzzle.h"
#include "ui_puzzle.h"
#include "puzzleselectdialog.h"
#include <QMessageBox>

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
    emit switchToWebcam();   // main.cpp 에서 이걸 받아서 페이지 전환
}


void puzzle::on_imageButton_clicked()
{

}

