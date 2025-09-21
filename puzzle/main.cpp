#include "puzzle.h"
#include "webcam_capture.h"   // 클래스 include
#include "make_puzzle_image.h"

#include <QApplication>
#include <QStackedWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QStackedWidget stacked;

    // Puzzle 화면
    puzzle *puzzlePage = new puzzle;

    // Webcam 화면 (Ui::webcam_capture 대신 클래스 사용)
    WebcamCapture *webcamPage = new WebcamCapture;

    // 퍼즐 만들기 화면
    makePuzzleImage *makePuzzlePage = new makePuzzleImage;

    // 스택에 추가
    stacked.addWidget(puzzlePage);      // index 0
    stacked.addWidget(webcamPage);      // index 1
    stacked.addWidget(makePuzzlePage);  // index 2

    QObject::connect(puzzlePage, &puzzle::switchToWebcam, [&]() {
        stacked.setCurrentIndex(1);
    });

    stacked.setCurrentIndex(0);
    stacked.resize(1024, 768);
    stacked.show();

    return a.exec();
}
