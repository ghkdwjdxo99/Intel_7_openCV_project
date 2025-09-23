#include "puzzle.h"
#include "webcam_capture.h"
#include "make_puzzle_image.h"
#include "puzzleselectdialog.h"
#include "playpage.h"
#include "successdialog.h"
#include "faildialog.h"

#include "puzzleboard.h"

#include <QApplication>
#include <QStackedWidget>

#define QT_DEBUG

int g_puzzleType = 0;   // 퍼즐 타입 저장

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QStackedWidget stacked;

    // 현재 선택된 퍼즐 타입 (기본 5로 초기화)
    int currentPuzzleType = 5;

    // 페이지들
    puzzle *puzzlePage = new puzzle;
    WebcamCapture *webcamPage = new WebcamCapture;
    makePuzzleImage *makePuzzlePage = new makePuzzleImage;
    PlayPage *playPage = new PlayPage;
    SuccessDialog *successDlg = new SuccessDialog;
    FailDialog *failDlg = new FailDialog;

    stacked.addWidget(puzzlePage);      // 0
    stacked.addWidget(webcamPage);      // 1
    stacked.addWidget(makePuzzlePage);  // 2
    stacked.addWidget(playPage);        // 3

    // 퍼즐 페이지 -> 웹캠 페이지 (퍼즐 타입 전달받아 저장)
    QObject::connect(puzzlePage, &puzzle::switchToWebcam, [&](int puzzleType) {
        currentPuzzleType = puzzleType;
        makePuzzlePage->setPuzzleType(puzzleType);  // ★ 타입 먼저 세팅
        stacked.setCurrentIndex(1);
    });


    QObject::connect(webcamPage, &WebcamCapture::switchToMakePuzzle, [&]() {
        stacked.setCurrentIndex(2);
        makePuzzlePage->loadCapturedImage();        // ★ 무인자 호출 유지
    });

    // 퍼즐 종료 → 다이얼로그 …
    QObject::connect(playPage, &PlayPage::puzzleFinished,
                     [&](int elapsed, bool success){
        if (success) {
            SuccessDialog dlg(&stacked);
            dlg.setTime(elapsed);
            dlg.move(stacked.geometry().center() - dlg.rect().center());
            dlg.exec();
        } else {
            FailDialog dlg(&stacked);
            dlg.setTime(elapsed);
            dlg.move(stacked.geometry().center() - dlg.rect().center());
            dlg.exec();
        }
        stacked.setCurrentIndex(0);
    });

    QObject::connect(successDlg, &SuccessDialog::backToMain, [&]() {
        stacked.setCurrentIndex(0);
    });
    QObject::connect(failDlg, &FailDialog::backToMain, [&]() {
        stacked.setCurrentIndex(0);
    });

    QObject::connect(makePuzzlePage, &makePuzzleImage::showPlayPage, [&](){
        playPage->setPuzzleBoard(g_puzzleType);   // 기존 퍼즐 선택값 전달
        stacked.setCurrentWidget(playPage);
    });
    QObject::connect(playPage, &PlayPage::showPuzzle, [&](){
        stacked.setCurrentWidget(puzzlePage);
    });

    stacked.setCurrentIndex(0);
    stacked.resize(1024, 768);
    stacked.show();

    return a.exec();
}
