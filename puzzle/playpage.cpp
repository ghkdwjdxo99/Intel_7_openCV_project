#include "playpage.h"
#include "ui_playpage.h"
#include "puzzle.h"
#include "successdialog.h"
#include "faildialog.h"
#include "solutiondialog.h"
#include "puzzleboard.h"

#include <QStackedWidget>
#include <QMetaObject>
#include <QDebug>
#include <QTimer>
#include <QMessageBox>

PlayPage::PlayPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlayPage),
    timer(new QTimer(this)),
    elapsedSeconds(0)
{
    ui->setupUi(this);
    ui->PuzzleBoard->configure(/*rows*/8, /*cols*/8, QSizeF(640,640)); // 5x5면 5,5

    // 타이머와 슬롯 연결
    connect(timer, &QTimer::timeout, this, &PlayPage::updateTime);

    // 초기 라벨 값 설정
    ui->timerLabel->setText("00:00");

    // 힌트 횟수 초기화
    hintCount = 3;
    ui->HintBT->setText("힌트 (3회)");
}

PlayPage::~PlayPage()
{
    delete ui;
}

void PlayPage::on_StopBT_clicked()
{
    timer->stop();  // 타이머 멈춤

    // ✅ 테스트: 선택 시 SuccessDialog 띄우기
//    SuccessDialog dlg(this);
//    FailDialog dlg(this);
//    dlg.exec();

//    this->close();

//    emit showPuzzle();
    emit puzzleFinished(elapsedSeconds, false); //포기 시그널 발생
}

void PlayPage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    elapsedSeconds = 0;
    ui->timerLabel->setText("00:00");
    timer->start(1000);  // 1초마다 updateTime 실행

    hintCount = 3;
    ui->HintBT->setText("힌트 (3회)");
}

void PlayPage::updateTime()
{
    elapsedSeconds++;

    int minutes = elapsedSeconds / 60;
    int seconds = elapsedSeconds % 60;

    ui->timerLabel->setText(
        QString("%1:%2")
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'))
    );
}

void PlayPage::on_SolutionBT_clicked()
{
   SolutionDialog dlg(this);
   dlg.setImage("./images/capture_image/puzzle_image.png");  // 이미지 경로 전달
   dlg.exec();
}


void PlayPage::on_HintBT_clicked()
{
    if(hintCount > 0){
        hintCount--;
        ui->HintBT->setText(QString("힌트 (%1회)").arg(hintCount));

        // 실제 힌트 기능 실행
        qDebug() << "힌트 사용! 남은 횟수:" << hintCount;
    }

    else{
        QMessageBox::warning(this, "힌트 제한", "더 이상 힌트를 사용할 수 없습니다!!!");
    }
}

void PlayPage::setPuzzleBoard(int type)
{
    QString imgPath;
    if (type == 5)
        imgPath = "./images/puzzle_mask_5x5.png";
    else if (type == 8)
        imgPath = "./images/puzzle_mask_8x8.png";
    else
        return;

    QPixmap board(imgPath);
    if (board.isNull()) {
        qWarning() << "퍼즐 보드 이미지 불러오기 실패:" << imgPath;
        return;
    }

    // QGraphicsScene을 만들어서 QGraphicsView에 세팅
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->addPixmap(board.scaled(
        ui->PuzzleBoard->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    ));
    ui->PuzzleBoard->setScene(scene);
}


