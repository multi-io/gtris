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

    BrickViewer *getPlayField() const;
    BrickViewer *getNextField() const;

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

protected:
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *event);
};

#endif // MAINWINDOW_H
