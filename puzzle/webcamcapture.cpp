#include "webcamcapture.h"
#include "ui_webcamcapture.h"

WebcamCapture::WebcamCapture(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WebcamCapture)
{
    ui->setupUi(this);
}

WebcamCapture::~WebcamCapture()
{
    delete ui;
}
