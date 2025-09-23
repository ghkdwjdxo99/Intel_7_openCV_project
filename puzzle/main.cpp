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

#include <QFile>
#include <QFontDatabase>

#include <QFileDialog>    // [ADDED]
#include <QDir>           // [ADDED]
#include <QMessageBox>    // [ADDED]
#include <QCoreApplication> // [ADDED]

#define QT_DEBUG

int g_puzzleType = 0;   // 퍼즐 타입 저장

// (선택) 복사 유틸 - 캡처 저장 경로로 강제 복사
static bool copyToCaptureJpg(const QString& src, QString* outDst = nullptr)  // [ADDED]
{
    const QString capDir = QCoreApplication::applicationDirPath() + "/images/capture_image"; // [ADDED]
    QDir().mkpath(capDir);                                                                     // [ADDED]
    const QString dst = capDir + "/capture_image.jpg";                                         // [ADDED]
    if (QFile::exists(dst)) QFile::remove(dst);                                                // [ADDED]
    if (!QFile::copy(src, dst)) return false;                                                  // [ADDED]
    if (outDst) *outDst = dst;                                                                 // [ADDED]
    return true;                                                                               // [ADDED]
}

static void loadStyle() {
    QFontDatabase::addApplicationFont(":/fonts/NotoSansKR-Regular.otf");
    qApp->setFont(QFont("Noto Sans KR", 11));

    QFile f(":/qss/app.qss");
    if (f.open(QIODevice::ReadOnly)) {
        qApp->setStyleSheet(QString::fromUtf8(f.readAll()));
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    loadStyle();

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

    // ① 메인창 "이미지 불러오기" → (여기서) 파일선택/복사 → 위치조절창 전환
    QObject::connect(puzzlePage, &puzzle::imageImportRequested,    // [ADDED]
                     [&](int type){                                // [ADDED]
        // 2) 파일 열기
        const QString src = QFileDialog::getOpenFileName(
            &stacked,
            QObject::tr("이미지 선택"),
            QDir::homePath(),
            QObject::tr("Images (*.png *.jpg *.jpeg *.bmp)"));     // [ADDED]

        if (src.isEmpty()) return;                                 // [ADDED]

        // 3) 지정 경로로 복사: ./images/capture_image/capture_image.jpg
        QString dst;                                               // [ADDED]
        if (!copyToCaptureJpg(src, &dst)) {                        // [ADDED]
            QMessageBox::warning(&stacked, QObject::tr("오류"),
                                 QObject::tr("이미지 복사 실패"));  // [ADDED]
            return;                                                // [ADDED]
        }

        // 4) 퍼즐 위치 조절 페이지 세팅
        //    - 퍼즐 타입 전달 (이미 구현돼있다고 했으므로 그대로 사용)
        makePuzzlePage->setPuzzleType(type);                       // [ADDED] 5 또는 8

        //    - 배경 이미지 로드(기존 로더 호출) : 너희 코드에 이미 있는 함수 호출
        //      예) loadCapturedImage()가 capture_image.jpg를 읽어들이도록 구현되어 있음
        makePuzzlePage->loadCapturedImage();                       // [ADDED] (기존 함수명 그대로 사용)

        // 5) 전환
        stacked.setCurrentWidget(makePuzzlePage);                  // [ADDED]
    });


    stacked.setCurrentIndex(0);
    stacked.resize(1024, 768);
    stacked.show();

    return a.exec();
}
