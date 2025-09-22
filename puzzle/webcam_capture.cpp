#include "webcam_capture.h"
#include "ui_webcam_capture.h"
#include <QDebug>


#include <QImage>
#include <QPixmap>


#include <QDir>
#include <QDateTime>

WebcamCapture::WebcamCapture(QWidget *parent)
    : QWidget(parent), ui(new Ui::webcam_capture)
{
    ui->setupUi(this);

    cap.open(0);
    if (!cap.isOpened()) {
        ui->video_label->setText("웹캠 열기 실패!");
        return;
    }

    connect(&timer, &QTimer::timeout, this, &WebcamCapture::updateFrame);
    timer.start(33); // 약 30fps

    connect(ui->capture_btn, &QPushButton::clicked, this, &WebcamCapture::on_capture_btn_clicked);
}

WebcamCapture::~WebcamCapture() {
    cap.release();
    delete ui;
}

void WebcamCapture::updateFrame()
{
    cap >> frame;
    if (frame.empty()) return;

    // 1. 원본 frame을 복사해서 변환
    cv::Mat rgbFrame;
    cv::cvtColor(frame, rgbFrame, cv::COLOR_BGR2RGB);

    QImage qimg(rgbFrame.data, rgbFrame.cols, rgbFrame.rows, rgbFrame.step, QImage::Format_RGB888);
    ui->video_label->setPixmap(QPixmap::fromImage(qimg).scaled(
        ui->video_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void WebcamCapture::on_capture_btn_clicked()
{
    if (!frame.empty()) {
            // 1. 저장 폴더 경로 지정 (상대경로)
            QString saveDir = "../puzzle/images/chapture_image";
            QDir().mkpath(saveDir);  // 폴더 없으면 생성

            // 2. 파일명 (타임스탬프 붙이면 중복 안 됨)
            QString filename = saveDir + QString("/capture_%1.jpg")
                .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

            // 3. 저장
            cv::imwrite(filename.toStdString(), frame);
        }


    qDebug() << "촬영됨";

    emit switchToMakePuzzle();   // 선택값 전달
}

