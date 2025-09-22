#include "successdialog.h"
#include "ui_successdialog.h"

SuccessDialog::SuccessDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SuccessDialog)
{
    ui->setupUi(this);
}

SuccessDialog::~SuccessDialog()
{
    delete ui;
}

void SuccessDialog::on_pushButton_clicked()
{
    emit backToMain();  // 신호 발생
    accept();           // 다이얼로그 닫기
}

