#ifndef MAKEPUZZLEIMAGE_H
#define MAKEPUZZLEIMAGE_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class make_puzzle_image; }
QT_END_NAMESPACE

class makePuzzleImage : public QWidget
{
    Q_OBJECT

public:
    makePuzzleImage(QWidget *parent = nullptr);
    ~makePuzzleImage();

private:
    Ui::make_puzzle_image *make_puzzle_image_ui;
};

#endif // MAKEPUZZLEIMAGE_H
