#include "webcam_capture.h"
#include "ui_webcam_capture.h"
#include <QDebug>

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

void WebcamCapture::on_capture_btn_clicked()
{
    qDebug() << "촬영됨";

    emit switchToMakePuzzle();   // 선택값 전달
}

