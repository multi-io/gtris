#include "mainwindow.h"
#include "TetrisGameProcess.h"
#include "HighscoresManager.h"

#include <QAction>
#include <QMessageBox>
#include <QApplication>
#include <QTimer>
#include <QtWidgets>
#include <QDir>
#include <QDebug>
#include <ctime>
#include <cmath>

static MainWindow *mainWnd;
static QTimer timer;
static TetrisGameProcess *gameProc;
static HighscoresManager *hscManager;

static int currLevel, currSpeed;
static bool gamePaused = false;
static time_t latestGameResumeTime;
static int gameTimeBeforeLatestResume;


static int getGameTime() {
    return gameTimeBeforeLatestResume + (gamePaused ? 0 : (time(0) - latestGameResumeTime));
}


//in level 5 (nLevels-1): speed[gameTime] := speed_c * t ^ speed_ex

static const double speed_ex = 0.5;   //req'd 0<ex<=1. Smaller ex means less time spent at lower speeds and more at higher speeds. Greater ex smoothens things out. (ex=1 => linear case, same time at all speeds)
static const double speed_l4 = 300;   //time spent at speed 4

static const double speed_c = pow(1.0/speed_l4 * (pow(5, (1.0/speed_ex)) - pow(4, (1.0/speed_ex))), speed_ex);

static int getSpeed(int level, int gameTime) {
    if (level < nLevels - 1) {
        return level;
    } else {
        return speed_c * pow(gameTime, speed_ex);
    }
}

static int getCurrentlyNeededSpeed() {
    return getSpeed(currLevel, getGameTime());
}


// timeMsec[speed] := t0*exp(f*speed)

static const double t0 = 330.0, t4 = 110.0;  //required timerMsec values at speed 0 and 4, respectively (for backwards compat.)
static const double f = 0.25 * log(t4/t0);

static int getTimerMsec(int speed) {
    return t0 * exp(f * speed);
}

static void startOrAdjustTimerAccordingToCurrSpeed() {
    timer.start(getTimerMsec(currSpeed));
}

static void updateStatusDisplay() {
    static int prevLevel = -1, prevScore = -1, prevSpeed = -1;
    if (currLevel != prevLevel) {
        mainWnd->displayLevel(currLevel);
        prevLevel = currLevel;
    }
    if (prevSpeed != currSpeed) {
        mainWnd->displaySpeed(currSpeed);
        prevSpeed = currSpeed;
    }
    if (prevScore != gameProc->GetScore()) {
        mainWnd->displayScore(gameProc->GetScore());
        mainWnd->displayLines(gameProc->GetLines());
        prevScore = gameProc->GetScore();
    }
}

static void timerTick() {
    static std::string prevName = "Homer";
    if (gamePaused) {
        return;
    }
    if (!gameProc->StepForth()) {
        timer.stop();
        int score = gameProc->GetScore();
        if (score > hscManager->getLeastScore(currLevel)) {
            HscEntry e(prevName.c_str(), gameProc->GetScore(), gameProc->GetLines(), time(0));
            if (hscManager->highscoresUserQuery(&e, 4)) {
                hscManager->addNewEntry(e, currLevel);
                prevName = e.name;
            } else {
                printf("dialog aborted.");
            }
        }
    }
    //adjust speed if needed
    int newSpeed = getCurrentlyNeededSpeed();
    if (newSpeed != currSpeed) {
        currSpeed = newSpeed;
        startOrAdjustTimerAccordingToCurrSpeed();
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
    currSpeed = getCurrentlyNeededSpeed();
    startOrAdjustTimerAccordingToCurrSpeed();
    gameProc->StartNewGame();
    gamePaused = false;
    latestGameResumeTime = time(0);
    gameTimeBeforeLatestResume = 0;
    updateStatusDisplay();
}

static void runGameAction() {
    if (!gameProc->IsGameRunning()) {
        newGameAction();
    } else if (gamePaused) {
        gamePaused = false;
        latestGameResumeTime = time(0);
    }
}

static void pauseGameAction() {
    gameTimeBeforeLatestResume = getGameTime();
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
    if (!gamePaused) {
        gameProc->ProcessKey(key);
    }
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
    hm.setBackingFile(QDir::home().filePath(".gtrisscores.bin").toUtf8().constData());

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
