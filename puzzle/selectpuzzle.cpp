#include "selectpuzzle.h"
#include "ui_selectpuzzle.h"

SelectPuzzle::SelectPuzzle(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SelectPuzzle)
{
    ui->setupUi(this);
}

SelectPuzzle::~SelectPuzzle()
{
    delete ui;
}
