#ifndef SOLUTIONDIALOG_H
#define SOLUTIONDIALOG_H

#include <QDialog>

namespace Ui {
class SolutionDialog;
}

class SolutionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SolutionDialog(QWidget *parent = nullptr);
    ~SolutionDialog();

    void setImage(const QString &path);     // 원본 이미지 경로를 받아 QLabel에 띄우는 함수

private:
    Ui::SolutionDialog *ui;
};

#endif // SOLUTIONDIALOG_H
