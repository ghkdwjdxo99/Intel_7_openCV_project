#include "puzzle.h"
#include "ui_puzzle.h"
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
    QMessageBox::information(this, "Puzzle Game", "📸 사진 촬영 버튼 클릭됨!");
}

void puzzle::on_imageButton_clicked()
{
    QMessageBox::information(this, "Puzzle Game", "🖼️ 이미지 불러오기 버튼 클릭됨!");
}
