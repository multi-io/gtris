#include "mainwindow.h"
#include "TetrisGameProcess.h"
#include "HighscoresManager.h"

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
static HighscoresManager *hscManager;

static int currLevel, currSpeed;


static void timerTick() {
    if (!gameProc->StepForth()) {
        timer.stop();
    }
}


static void newGame() {
}

static void runGame() {
}

static void stopGame() {
}

static void pauseGame() {
}

static void canClose(bool *canClose) {
    //TODO user query if game running
    *canClose = true;
}

static void exitGame() {
    bool cc = true;
    canClose(&cc);
    if (cc) {
        QApplication::quit();
    }
}

static void toggleHighscores() {
    hscManager->showDialog(!hscManager->isDialogVisible());
}

static void showAbout() {
}

static void showOptions() {
}

static void processKey(int key) {
    gameProc->ProcessKey(key);
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    gameProc = new TetrisGameProcess(w.getPlayField(), w.getNextField());
    hscManager = new HighscoresManager(&w);
    hscManager->addNewEntry(HscEntry("Olaf", 211233, 10, 88923), 0);
    hscManager->addNewEntry(HscEntry("Joe", 202995, 12, 88924), 0);
    hscManager->addNewEntry(HscEntry("Suzy", 1788, 7, 98920), 0);
    hscManager->addNewEntry(HscEntry("Lenny", 232995, 12, 88924), 0);

    hscManager->addNewEntry(HscEntry("Joe", 89932, 12, 3388924), 4);
    hscManager->addNewEntry(HscEntry("Suzy", 93984, 7, 5698920), 4);
    hscManager->addNewEntry(HscEntry("Lenny", 25653, 12, 1988924), 4);

    for (int i = 1000; i < 100000; i += 1000) {
        hscManager->addNewEntry(HscEntry("Jonny", i, i/10, time(0)), 3);
    }

    printf("%i\n%i\n%i\n", hscManager->getLeastScore(0), hscManager->getLeastScore(1), hscManager->getLeastScore(3));

    QObject::connect(w.actionNew, &QAction::triggered, &newGame);
    QObject::connect(w.actionRun, &QAction::triggered, &runGame);
    QObject::connect(w.actionStop, &QAction::triggered, &stopGame);
    QObject::connect(w.actionPause, &QAction::triggered, &pauseGame);
    QObject::connect(w.actionExit, &QAction::triggered, &exitGame);
    QObject::connect(w.actionHighscores, &QAction::triggered, &toggleHighscores);
    QObject::connect(w.actionAbout, &QAction::triggered, &showAbout);
    QObject::connect(w.actionGame_Options, &QAction::triggered, &showOptions);


    //QObject::connect(&w, &MainWindow::keyPressed, gameProc, &TetrisGameProcess::ProcessKey);
    //QObject::connect(&w, &MainWindow::keyPressed, std::bind(&TetrisGameProcess::ProcessKey, *gameProc, std::placeholders::_1));
    QObject::connect(&w, &MainWindow::keyPressed, &processKey);

    QObject::connect(&w, &MainWindow::canClose, &canClose /*, Qt::DirectConnection*/);

    w.show();

    QObject::connect(&timer, &QTimer::timeout, &timerTick);
    timer.start(500);
    gameProc->StartNewGame();

    return a.exec();
}
