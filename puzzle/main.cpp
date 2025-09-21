#include "puzzle.h"
#include "webcam_capture.h"

#include <QApplication>
#include <QStackedWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QStackedWidget stacked;

//    puzzle w;
//    w.show();

    // 각 페이지 생성
    puzzle *puzzlePage = new puzzle;
    WebcamCapture *webcamPage = new WebcamCapture;

    // 스택에 추가
    stacked.addWidget(puzzlePage);  // index 0
    stacked.addWidget(webcamPage);  // index 1

    // 페이지 전환 연결
    QObject::connect(puzzlePage, &puzzle::switchToWebcam, [&]() {
        stacked.setCurrentIndex(1);
    });

    QObject::connect(webcamPage, &WebcamCapture::switchToPuzzle, [&]() {
        stacked.setCurrentIndex(0);
    });

    // 초기 화면
    stacked.setCurrentIndex(0);
    stacked.resize(1024, 768);
    stacked.show();


    return a.exec();
}
