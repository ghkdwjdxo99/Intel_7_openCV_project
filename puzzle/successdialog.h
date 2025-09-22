#ifndef SUCCESSDIALOG_H
#define SUCCESSDIALOG_H

#include <QDialog>

namespace Ui {
class SuccessDialog;
}

class SuccessDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SuccessDialog(QWidget *parent = nullptr);
    ~SuccessDialog();

signals:
    void backToMain();  // 메인으로 돌아가라는 시그널

private slots:
    void on_pushButton_clicked();

private:
    Ui::SuccessDialog *ui;
};

#endif // SUCCESSDIALOG_H
