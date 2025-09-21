#include "puzzle.h"
#include "playpage.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    puzzle w;
     PlayPage w;
    w.show();
    return a.exec();
}
