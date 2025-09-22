#include "puzzle.h"

#include "webcam_capture.h"   // 클래스 include
#include "make_puzzle_image.h"
#include "puzzleselectdialog.h"
#include "playpage.h"
#include "successdialog.h"
#include "faildialog.h"

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

    // play 화면
    PlayPage *playPage = new PlayPage;

    // success Dialog
    SuccessDialog *successDlg = new SuccessDialog;

    // fail Dialog
    FailDialog *failDlg = new FailDialog;

    // 스택에 추가
    stacked.addWidget(puzzlePage);      // index 0
    stacked.addWidget(webcamPage);      // index 1
    stacked.addWidget(makePuzzlePage);  // index 2
    stacked.addWidget(playPage);        // index 3

    QObject::connect(puzzlePage, &puzzle::switchToWebcam, [&](int puzzleType) {
        stacked.setCurrentIndex(1);   // webcamCapture 화면으로 이동

        // 필요하다면 webcamPage에 선택값 전달하는 로직 추가 가능
    });

    QObject::connect(webcamPage, &WebcamCapture::switchToMakePuzzle, [&]() {
        stacked.setCurrentIndex(2);   // webcamCapture 화면으로 이동
    });

    // 퍼즐 종료 시 → 다이얼로그 띄우기
    QObject::connect(playPage, &PlayPage::puzzleFinished,
                     [&](int elapsed, bool success){
        if (success) {
            SuccessDialog dlg(&stacked);
            dlg.setTime(elapsed);  // ⬅️ 걸린 시간 전달

            // 부모 중심으로 이동 보정 (필요 시)
            dlg.move(stacked.geometry().center() - dlg.rect().center());

            dlg.exec();
        } else {
            FailDialog dlg(&stacked);
//            SuccessDialog dlg(&stacked);
            dlg.setTime(elapsed);  // ⬅️ 걸린 시간 전달

            dlg.move(stacked.geometry().center() - dlg.rect().center());

            dlg.exec();
        }
        stacked.setCurrentIndex(0); // 메인으로 복귀
    });

    // successdialog 연결 추가
    QObject::connect(successDlg, &SuccessDialog::backToMain, [&]() {
        stacked.setCurrentIndex(0);   // 메인 화면(Page0)으로 전환
    });

    // faildialog 연결 추가
    QObject::connect(failDlg, &FailDialog::backToMain, [&]() {
        stacked.setCurrentIndex(0);   // 메인 화면(Page0)으로 전환
    });




    QObject::connect(makePuzzlePage, &makePuzzleImage::showPlayPage, [&](){
        stacked.setCurrentWidget(playPage);
    });

//    QObject::connect(playPage, &PlayPage::showPuzzle, [&](){
//        stacked.setCurrentWidget(puzzlePage);
//    });

    QObject::connect(playPage, &PlayPage::showPuzzle, [&](){
//        stacked.setCurrentWidget(puzzlePage);
        stacked.setCurrentWidget(puzzlePage);
    });

    stacked.setCurrentIndex(0);
    stacked.resize(1024, 768);
    stacked.show();

    return a.exec();
}
