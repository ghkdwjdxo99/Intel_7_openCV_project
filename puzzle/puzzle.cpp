#include "puzzle.h"
#include "ui_puzzle.h"
#include "puzzleselectdialog.h"
#include <QMessageBox>
#include <QDebug>
#include <QDir>


puzzle::puzzle(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::puzzle)
{
    ui->setupUi(this);
}

puzzle::~puzzle()
{
    delete ui;
}

void puzzle::on_cameraButton_clicked()
{
    PuzzleSelectDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        int type = dlg.selectedPuzzleType();
        qDebug() << "카메라 버튼 → 선택된 퍼즐 타입:" << type;
        clearFolder("../puzzle/images/capture_image");
        clearFolder("../puzzle/images/piece_image");

        emit switchToWebcam(type);   // 선택값 전달

        // 필요하다면 메시지박스 대신 스택 전환만 하도록 수정
        /*
        if (type == 5) {
            QMessageBox::information(this, "선택 결과", "5x5 퍼즐 선택됨!");
        } else if (type == 8) {
            QMessageBox::information(this, "선택 결과", "8x8 퍼즐 선택됨!");
        }
        */
    }
}


void puzzle::on_imageButton_clicked()
{
    clearFolder("../puzzle/images/capture_image");
    clearFolder("../puzzle/images/piece_image");

    PuzzleSelectDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        int type = dlg.selectedPuzzleType();
        qDebug() << "이미지 버튼 → 선택된 퍼즐 타입:" << type;

        if (type == 5) {
            QMessageBox::information(this, "선택 결과", "5x5 퍼즐 선택됨!");
        } else if (type == 8) {
            QMessageBox::information(this, "선택 결과", "8x8 퍼즐 선택됨!");
        }
    }

    const int type = dlg.selectedPuzzleType();    // [ADDED] 5 또는 8 (이미 구현돼있는 함수라 가정)
    emit imageImportRequested(type);        // [ADDED]  -> 파일 선택/복사는 main.cpp에서


}



// 이 함수는 지정 폴더 안의 파일을 모두 삭제하고, 파일명을 출력합니다.
void puzzle::clearFolder(const QString &folderPath)
{
    QDir dir(folderPath);
    if (!dir.exists()) return;

    dir.setFilter(QDir::Files);
    QStringList files = dir.entryList();

    for (const QString &file : files) {
        QString filePath = dir.absoluteFilePath(file);
        if (dir.remove(file)) {
            qDebug() << "삭제 성공:" << filePath;
        } else {
            qDebug() << "삭제 실패:" << filePath;
        }
    }
}
