#include "mainwindow.h"
#include "TetrisGameProcess.h"
#include "HighscoresManager.h"

#include <QAction>
#include <QMessageBox>
#include <QApplication>
#include <QTimer>
#include <QtWidgets>

static MainWindow *mainWnd;
static QTimer timer;
static TetrisGameProcess *gameProc;
static HighscoresManager *hscManager;

static int currLevel, currSpeed;
static bool gamePaused = false;

static void updateStatusDisplay() {
    static int prevLevel = -1, prevScore = -1;
    if (currLevel != prevLevel) {
        mainWnd->displayLevel(currLevel);
        prevLevel = currLevel;
    }
    if (prevScore != gameProc->GetScore()) {
        mainWnd->displayScore(gameProc->GetScore());
        mainWnd->displayLines(gameProc->GetLines());
        prevScore = gameProc->GetScore();
    }
}

static void timerTick() {
    if (!gameProc->StepForth()) {
        timer.stop();
        int score = gameProc->GetScore();
        if (score > hscManager->getLeastScore(currLevel)) {
            HscEntry e("Homer", gameProc->GetScore(), gameProc->GetLines(), time(0));
            if (hscManager->highscoresUserQuery(&e, 4)) {
                hscManager->addNewEntry(e, currLevel);
            } else {
                printf("dialog aborted.");
            }
        }
    }
    updateStatusDisplay();
}

static bool query(const char *text, const char *infText) {
    QMessageBox msgBox;
    msgBox.setText(text);
    msgBox.setInformativeText(infText);
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Ok);
    return (QMessageBox::Ok == msgBox.exec());
}

static void newGameAction() {
    if (gameProc->IsGameRunning()) {
        if (!query("Game in Progress", "Game still running. Proceed?")) {
            return;
        }
    }
    currLevel = mainWnd->getSelectedLevel();
    timer.start(55 * (nLevels+1 - currLevel));
    gameProc->StartNewGame();
    gamePaused = false;
    updateStatusDisplay();
}

static void runGameAction() {
    if (!gameProc->IsGameRunning()) {
        newGameAction();
    } else if (gamePaused) {
        timer.start(55 * (nLevels+1 - currLevel));
        gamePaused = false;
    }
}

static void pauseGameAction() {
    timer.stop();
    gamePaused = true;
}

static void canClose(bool *canClose) {
    if (gameProc->IsGameRunning()) {
        *canClose = query("Game in Progress", "Game still running. Proceed?");
    }
}

static void stopGameAction() {
    bool cc = true;
    canClose(&cc);
    if (cc) {
        gameProc->StopGame();
    }
}

static void exitGameAction() {
    bool cc = true;
    canClose(&cc);
    if (cc) {
        QApplication::quit();
    }
}

static void toggleHighscoresAction() {
    hscManager->showDialog(!hscManager->isDialogVisible());
}

static void showAboutAction() {
}

static void showOptionsAction() {
}

static void processKey(int key) {
    gameProc->ProcessKey(key);
    updateStatusDisplay();
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    mainWnd = &w;

    TetrisGameProcess tgp(mainWnd->getPlayField(), mainWnd->getNextField());
    gameProc = &tgp;

    HighscoresManager hm(mainWnd);
    hscManager = &hm;

    QObject::connect(mainWnd->actionNew, &QAction::triggered, &newGameAction);
    QObject::connect(mainWnd->actionRun, &QAction::triggered, &runGameAction);
    QObject::connect(mainWnd->actionStop, &QAction::triggered, &stopGameAction);
    QObject::connect(mainWnd->actionPause, &QAction::triggered, &pauseGameAction);
    QObject::connect(mainWnd->actionExit, &QAction::triggered, &exitGameAction);
    QObject::connect(mainWnd->actionHighscores, &QAction::triggered, &toggleHighscoresAction);
    QObject::connect(mainWnd->actionAbout, &QAction::triggered, &showAboutAction);
    QObject::connect(mainWnd->actionGame_Options, &QAction::triggered, &showOptionsAction);
    //QObject::connect(&w, &MainWindow::levelChosen, &levelSelected);

    //QObject::connect(&w, &MainWindow::keyPressed, gameProc, &TetrisGameProcess::ProcessKey);
    //QObject::connect(&w, &MainWindow::keyPressed, std::bind(&TetrisGameProcess::ProcessKey, *gameProc, std::placeholders::_1));
    QObject::connect(mainWnd, &MainWindow::keyPressed, &processKey);

    QObject::connect(mainWnd, &MainWindow::canClose, &canClose /*, Qt::DirectConnection*/);

    mainWnd->show();

    QObject::connect(&timer, &QTimer::timeout, &timerTick);

    return a.exec();
}
