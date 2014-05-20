#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <memory>
#include "BrickViewer.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    Ui::MainWindow * const ui;
    BrickViewer * /*const*/ m_playField,  * /*const*/ m_nextField;
    QLabel *m_scoreLabel, *m_linesLabel, *m_levelLabel, *m_speedLabel;
    int m_selectedLevel;

private slots:
    void levelChosen(int level);

public:
    explicit MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

    BrickViewer *getPlayField() const;
    BrickViewer *getNextField() const;

    void displayScore(int score);
    void displayLines(int lines);
    void displayLevel(int level);
    void displaySpeed(int speed);

    int getSelectedLevel() { return m_selectedLevel; }

    QAction * /*const*/ actionNew;
    QAction * /*const*/ actionRun;
    QAction * /*const*/ actionStop;
    QAction * /*const*/ actionPause;
    QAction * /*const*/ actionExit;
    QAction * /*const*/ actionHighscores;
    QAction * /*const*/ actionAbout;
    QAction * /*const*/ actionGame_Options;

signals:
    void keyPressed(int key);
    void canClose(bool *canClose);

protected:
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void closeEvent(QCloseEvent *event);
};

#endif // MAINWINDOW_H
