#include "playpage.h"
#include "ui_playpage.h"
#include "puzzle.h"
#include "successdialog.h"
#include "faildialog.h"

#include <QStackedWidget>
#include <QMetaObject>
#include <QDebug>
#include <QTimer>

PlayPage::PlayPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlayPage),
    timer(new QTimer(this)),
    elapsedSeconds(0)
{
    ui->setupUi(this);

    // 타이머와 슬롯 연결
    connect(timer, &QTimer::timeout, this, &PlayPage::updateTime);

    // 초기 라벨 값 설정
    ui->timerLabel->setText("00:00");
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
