#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "BrickViewer.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    Ui::MainWindow * const ui;
    BrickViewer * /*const*/ m_playField,  * /*const*/ m_nextField;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    BrickViewer *getPlayField();

    QAction * /*const*/ actionNew;
    QAction * /*const*/ actionRun;
    QAction * /*const*/ actionStop;
    QAction * /*const*/ actionPause;
    QAction * /*const*/ actionExit;
    QAction * /*const*/ actionHighscores;
    QAction * /*const*/ actionAbout;
    QAction * /*const*/ actionGame_Options;


protected:
    void resizeEvent(QResizeEvent *event);
};

#endif // MAINWINDOW_H
