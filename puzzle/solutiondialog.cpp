#include "solutiondialog.h"
#include "ui_solutiondialog.h"
#include <QPixmap>

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

// QLabel에 이미지 띄우기
void SolutionDialog::setImage(const QString &path)
{
    QPixmap pix(path);
    if (!pix.isNull()) {
        ui->label->setScaledContents(true);              // ⭐ 추가
        ui->label->setPixmap(pix);
        ui->label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    }
}
