#include "puzzle_select_qmsgbox.h"
#include "ui_puzzle_select_qmsgbox.h"
//#include "puzzle.h"
#include <QMessageBox>

PuzzleSelectQMsgBox::PuzzleSelectQMsgBox(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PuzzleSelectQMsgBox)
{
    ui->setupUi(this);
}

PuzzleSelectQMsgBox::~PuzzleSelectQMsgBox()
{
    delete ui;
}

void PuzzleSelectQMsgBox::on_cameraButton_clicked()
{
    QMessageBox::information(this, "Puzzle Game", "📸 사진 촬영 버튼 클릭됨!");
}

void PuzzleSelectQMsgBox::on_imageButton_clicked()
{
    QMessageBox::information(this, "Puzzle Game", "🖼️ 이미지 불러오기 버튼 클릭됨!");
}
