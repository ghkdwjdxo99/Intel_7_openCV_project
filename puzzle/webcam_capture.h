#ifndef WEBCAMCAPTURE_H
#define WEBCAMCAPTURE_H

#include <QWidget>
#include <QTimer>
#include <opencv2/opencv.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class webcam_capture; }
QT_END_NAMESPACE

class WebcamCapture : public QWidget
{
    Q_OBJECT

public:
    WebcamCapture(QWidget *parent = nullptr);
    ~WebcamCapture();

signals:
    void switchToMakePuzzle();    // 5 or 8 send

private slots:
    void updateFrame();
    void on_capture_btn_clicked();

private:
    Ui::webcam_capture *ui;
    QTimer timer;
    cv::VideoCapture cap;
    cv::Mat frame;
};

#endif // WEBCAMCAPTURE_H
