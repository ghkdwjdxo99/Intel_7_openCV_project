#include "faildialog.h"
#include "ui_faildialog.h"

FailDialog::FailDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FailDialog)
{
    ui->setupUi(this);
}

FailDialog::~FailDialog()
{
    delete ui;
}

void FailDialog::on_mainButton_clicked()
{
    emit backToMain();  // 신호 발생
    accept();           // 다이얼로그 닫기
}

void FailDialog::setTime(int seconds)
{
    int minutes = seconds / 60;
    int sec = seconds % 60;
    ui->timeLabel->setText(QString("걸린 시간 : %1분 %2초")
                           .arg(minutes)
                           .arg(sec));
}
