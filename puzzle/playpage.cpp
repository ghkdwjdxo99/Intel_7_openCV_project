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
    // 새 Puzzle 창을 띄우고 현재 PlayPage는 닫음.
    puzzle *p = new puzzle; // 또는 new Puzzle(nullptr)로 독립 창 생성
    p->show();

    this->close(); // 현재 페이지(창)를 닫음 — 진행 중 내용은 모두 버려짐
}
