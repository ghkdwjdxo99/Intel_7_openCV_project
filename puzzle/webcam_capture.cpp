#include "webcam_capture.h"
#include "ui_webcam_capture.h"

webcam_capture::webcam_capture(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::webcam_capture)
{
    ui->setupUi(this);
}

webcam_capture::~webcam_capture()
{
    delete ui;
}
