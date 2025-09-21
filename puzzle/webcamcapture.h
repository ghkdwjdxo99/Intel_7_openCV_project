#ifndef WEBCAMCAPTURE_H
#define WEBCAMCAPTURE_H

#include <QWidget>

namespace Ui {
class WebcamCapture;
}

class WebcamCapture : public QWidget
{
    Q_OBJECT

public:
    explicit WebcamCapture(QWidget *parent = nullptr);
    ~WebcamCapture();

private:
    Ui::WebcamCapture *ui;
};

#endif // WEBCAMCAPTURE_H
