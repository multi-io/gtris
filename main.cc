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
    //QObject::connect(&w, &MainWindow::keyPressed, gameProc, &TetrisGameProcess::ProcessKey);
    //QObject::connect(&w, &MainWindow::keyPressed, std::bind(&TetrisGameProcess::ProcessKey, *gameProc, std::placeholders::_1));
    QObject::connect(&w, &MainWindow::keyPressed, &processKey);

    w.show();
    hscManager->showDialog(true);

    HscEntry e("Homer", 98765, 43, time(0));
    if (hscManager->highscoresUserQuery(&e, 4)) {
        printf("name: %s  score: %i   lines: %i\n", e.name.c_str(), e.score, e.lines);
    } else {
        printf("dialog aborted.");
    }

    QObject::connect(&timer, &QTimer::timeout, &timerTick);
    timer.start(500);
    gameProc->StartNewGame();

    return a.exec();
}
