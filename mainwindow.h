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

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    BrickViewer *getPlayField();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    Ui::MainWindow * const ui;
    BrickViewer * /*const*/ m_playField,  * /*const*/ m_nextField;
};

#endif // MAINWINDOW_H
