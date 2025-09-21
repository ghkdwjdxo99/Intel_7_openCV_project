#ifndef WEBCAM_CAPTURE_H
#define WEBCAM_CAPTURE_H

#include <QDialog>

namespace Ui {
class webcam_capture;
}

class webcam_capture : public QDialog
{
    Q_OBJECT

public:
    explicit webcam_capture(QWidget *parent = nullptr);
    ~webcam_capture();

private:
    Ui::webcam_capture *ui;
};

#endif // WEBCAM_CAPTURE_H
