#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    BrickViewer *playField = w.getPlayField();
    playField->SetBrickColor(Qt::green, 2, 1);
    playField->SetBrickColor(Qt::lightGray, 2, 3);
    playField->SetBrickColor(Qt::red, 0, 0);
    playField->SetBrickColor(Qt::blue, playField->GetCols() - 1, playField->GetRows() - 1);

    return a.exec();
}
