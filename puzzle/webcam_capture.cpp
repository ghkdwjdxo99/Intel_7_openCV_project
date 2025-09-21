#include "webcam_capture.h"
#include "ui_webcam_capture.h"

WebcamCapture::WebcamCapture(QWidget *parent)
    : QWidget(parent)
    , webcam_capture_ui(new Ui::webcam_capture)
{
    webcam_capture_ui->setupUi(this);
}

WebcamCapture::~WebcamCapture()
{
    delete webcam_capture_ui;
}
