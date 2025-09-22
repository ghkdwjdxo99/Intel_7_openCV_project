#include "solutiondialog.h"
#include "ui_solutiondialog.h"

SolutionDialog::SolutionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SolutionDialog)
{
    ui->setupUi(this);
}

SolutionDialog::~SolutionDialog()
{
    delete ui;
}
