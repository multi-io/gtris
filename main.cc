#include "mainwindow.h"
#include "TetrisGameProcess.h"

#include <QAction>
#include <QMessageBox>
#include <QApplication>
#include <QTimer>
#include <QtWidgets>

#include <memory>
#include <functional>

static QTimer timer;
//static std::unique_ptr<TetrisGameProcess> gameProc;
static TetrisGameProcess *gameProc;

static void timerTick() {
    if (!gameProc->StepForth()) {
        timer.stop();
    }
}


static void newGame() {
    QMessageBox msgBox;
    msgBox.setText("newFile");
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

static void processKey(int key) {
    gameProc->ProcessKey(key);
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    gameProc = new TetrisGameProcess(w.getPlayField(), w.getNextField());
    QObject::connect(w.actionNew, &QAction::triggered, &newGame);

    //QObject::connect(&w, &MainWindow::keyPressed, gameProc, &TetrisGameProcess::ProcessKey);
    //QObject::connect(&w, &MainWindow::keyPressed, std::bind(&TetrisGameProcess::ProcessKey, *gameProc, std::placeholders::_1));
    QObject::connect(&w, &MainWindow::keyPressed, &processKey);

    w.show();
    QObject::connect(&timer, &QTimer::timeout, &timerTick);
    timer.start(500);
    gameProc->StartNewGame();

    return a.exec();
}
