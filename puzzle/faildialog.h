#ifndef FAILDIALOG_H
#define FAILDIALOG_H

#include <QDialog>

namespace Ui {
class FailDialog;
}

class FailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FailDialog(QWidget *parent = nullptr);
    ~FailDialog();
    void setTime(int seconds);  // 시간 전달 함수

signals:
    void backToMain();  // 메인으로 돌아가라는 시그널

private slots:
    void on_mainButton_clicked();

private:
    Ui::FailDialog *ui;
};

#endif // FAILDIALOG_H
