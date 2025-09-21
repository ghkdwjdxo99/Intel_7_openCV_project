#include "playpage.h"
#include "ui_playpage.h"
#include "puzzle.h"

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

void PlayPage::on_StBT_clicked()
{
    emit showPuzzle();
}
