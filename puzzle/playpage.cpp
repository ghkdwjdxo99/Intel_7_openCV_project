#include "playpage.h"
#include "ui_playpage.h"
#include "puzzle.h"
#include "successdialog.h"

#include <QStackedWidget>
#include <QMetaObject>
#include <QDebug>


PlayPage::PlayPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlayPage)
{
    ui->setupUi(this);
}

PlayPage::~PlayPage()
{
    delete ui;
}

void PlayPage::on_StopBT_clicked()
{
    // ✅ 테스트: 선택 시 SuccessDialog 띄우기
    SuccessDialog dlg(this);
    dlg.exec();

    this->close();

    emit showPuzzle();
}
