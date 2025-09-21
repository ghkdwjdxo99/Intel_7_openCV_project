#include "puzzle.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    puzzle w;
    w.show();
    return a.exec();
}
