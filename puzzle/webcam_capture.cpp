#include "webcam_capture.h"
#include "ui_webcam_capture.h"
#include <QDebug>
#include <QImage>
#include <QPixmap>
#include <QDir>
#include <QCoreApplication>

WebcamCapture::WebcamCapture(QWidget *parent)
    : QWidget(parent), ui(new Ui::webcam_capture)
{
    ui->setupUi(this);

    // 기본 카메라 열기
    cap.open(0, cv::CAP_V4L2);
    if (!cap.isOpened()) {
        ui->video_label->setText("웹캠 열기 실패!");
        return;
    }

    // 해상도 지정 (카메라가 지원하는 값으로 맞추세요)
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    double w = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    double h = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    qDebug() << "웹캠 해상도:" << w << "x" << h;

    // 주기적으로 프레임 업데이트 (약 30fps)
    connect(&timer, &QTimer::timeout, this, &WebcamCapture::updateFrame);
    timer.start(33);

    // 캡쳐 버튼 클릭 → 사진 저장 + 페이지 전환
    connect(ui->capture_btn, &QPushButton::clicked, this, &WebcamCapture::on_capture_btn_clicked);
}

WebcamCapture::~WebcamCapture() {
    if (cap.isOpened()) {
        cap.release();
        qDebug() << "카메라 종료";
    }
    delete ui;
}

void WebcamCapture::updateFrame()
{
    cap >> frame;
    if (frame.empty()) return;

    // BGR → RGB 변환
    cv::Mat rgbFrame;
    cv::cvtColor(frame, rgbFrame, cv::COLOR_BGR2RGB);

    // QImage 변환
    QImage qimg(rgbFrame.data, rgbFrame.cols, rgbFrame.rows,
                rgbFrame.step, QImage::Format_RGB888);

    // 라벨에 표시 (비율 유지, 크기 맞춤)
    ui->video_label->setPixmap(
        QPixmap::fromImage(qimg).scaled(
            ui->video_label->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        )
    );
}

void WebcamCapture::on_capture_btn_clicked()
{
    if (!frame.empty()) {
        // 저장 경로: <프로젝트>/puzzle/images/capture_image/capture_image.jpg
        QString saveDir = QCoreApplication::applicationDirPath() + "/images/capture_image";
        QDir().mkpath(saveDir);

        QString filename = saveDir + "/capture_image.jpg";

        // OpenCV로 저장
        bool success = cv::imwrite(filename.toStdString(), frame);
        if (success) {
            qDebug() << "촬영됨, 저장 경로:" << filename
                     << "크기:" << frame.cols << "x" << frame.rows;
        } else {
            qDebug() << "이미지 저장 실패!";
        }
    }

    // 페이지 전환 시그널
    emit switchToMakePuzzle();
}
