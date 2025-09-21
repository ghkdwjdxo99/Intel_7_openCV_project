#include "puzzle.h"
#include "ui_puzzle.h"
#include <QMessageBox>

puzzle::puzzle(QWidget *parent)
    : QWidget(parent)
    , main_ui(new Ui::puzzle)
{
    main_ui->setupUi(this);
}

puzzle::~puzzle()
{
    delete main_ui;
}

void puzzle::on_cameraButton_clicked()
{
    // 기존 QMessageBox 대신 화면 전환 시그널 emit
    emit switchToWebcam();
}

void puzzle::on_imageButton_clicked()
{
    QMessageBox::information(this, "Puzzle Game", "🖼️ 이미지 불러오기 버튼 클릭됨!");
}
