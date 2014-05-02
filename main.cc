#include "mainwindow.h"

#include <QAction>
#include <QMessageBox>
#include <QApplication>

static void newFile(bool b) {
    QMessageBox msgBox;
    msgBox.setText("newFile");
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    QObject::connect(w.actionNew, &QAction::triggered, newFile);
    //QObject::connect(w->actionOpen, &QAction::triggered, this, &MainController::openFile);
    //QObject::connect(m_mainWnd->actionExit, &QAction::triggered, [=](bool) { QApplication::exit(0); });  //TODO doesn't work -- unsupported in g++ 4.7?
    //QObject::connect(w->actionExit, &QAction::triggered, &QApplication::exit);


    BrickViewer *playField = w.getPlayField();
    playField->SetBrickColor(Qt::green, 2, 1);
    playField->SetBrickColor(Qt::lightGray, 2, 3);
    playField->SetBrickColor(Qt::red, 0, 0);
    playField->SetBrickColor(Qt::blue, playField->GetCols() - 1, playField->GetRows() - 1);

    return a.exec();
}
