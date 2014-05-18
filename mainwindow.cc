#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "HighscoresManager.h"  //for nLevels

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QSignalMapper>
#include <QDebug>

const QSize playFieldSize(12, 23);
const QSize nextFieldSize(5, 5);
const unsigned initialBrickSize = 20;

const QSize combinedSize(playFieldSize.width() + nextFieldSize.width(), std::max(playFieldSize.height(), nextFieldSize.height()));

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_scoreLabel(new QLabel),
    m_linesLabel(new QLabel),
    m_levelLabel(new QLabel),
    m_selectedLevel(0)
{
    ui->setupUi(this);
    actionNew = ui->actionNew;
    actionRun = ui->actionRun;
    actionStop = ui->actionStop;
    actionPause = ui->actionPause;
    actionExit = ui->actionExit;
    actionHighscores = ui->actionHighscores;
    actionAbout = ui->actionAbout;
    actionGame_Options = ui->actionGame_Options;
    QActionGroup *agrp = new QActionGroup(this);
    agrp->setExclusive(true);
    QSignalMapper *mapper = new QSignalMapper(this);
    for (int i = 0; i < nLevels; i++) {
        QAction *action = ui->menuOptions->addAction(QString("Level %1").arg(i));
        agrp->addAction(action);
        action->setCheckable(true);
        mapper->setMapping(action, i);
        //connect(action, &QAction::triggered, mapper, (void (QSignalMapper::*)()) &QSignalMapper::map);
        connect(action, SIGNAL(triggered(bool)), mapper, SLOT(map()));
    }
    qobject_cast<QAction*>(mapper->mapping(m_selectedLevel))->setChecked(true);
    //the SLOT macro only works with methods actually declared as "slots" in the header
    connect(mapper, SIGNAL(mapped(int)), this, SLOT(levelChosen(int)));

    QHBoxLayout *layout = new QHBoxLayout(ui->centralWidget);
    layout->setSpacing(1);
    layout->setMargin(1);
    //QWidgets (actually, all QObjects) own their children and thus auto-delete them
    layout->addWidget(m_playField = new BrickViewer(playFieldSize.width(), playFieldSize.height(), initialBrickSize));
    layout->addWidget(m_nextField = new BrickViewer(nextFieldSize.width(), nextFieldSize.height(), initialBrickSize));
    ui->centralWidget->setLayout(layout);
    statusBar()->addWidget(m_scoreLabel);
    statusBar()->addWidget(m_linesLabel);
    statusBar()->addWidget(m_levelLabel);
    displayScore(0);
    displayLines(0);
    displayLevel(0);
    adjustSize();
}

void MainWindow::levelChosen(int level) {
    qDebug() << "levelChosen: " << level << endl;
    m_selectedLevel = level;
}

void MainWindow::displayScore(int score) {
    m_scoreLabel->setText(QString("Score: %1").arg(score));
}

void MainWindow::displayLines(int lines) {
    m_linesLabel->setText(QString("Lines: %1").arg(lines));
}

void MainWindow::displayLevel(int level) {
    m_levelLabel->setText(QString("Level: %1").arg(level));
}

BrickViewer *MainWindow::getPlayField() const {
    return m_playField;
}

BrickViewer *MainWindow::getNextField() const {
    return m_nextField;
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::resizeEvent(QResizeEvent *) {
    int w = ui->centralWidget->width();
    int h = ui->centralWidget->height();
    int newBS = std::max(3, std::min(w / combinedSize.width(), h / combinedSize.height()));
    qDebug() << w << " " << h << " " << newBS << " " << endl;
    m_playField->SetBrickSize(newBS);
    m_nextField->SetBrickSize(newBS);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    emit this->keyPressed(event->key());
}

void MainWindow::closeEvent(QCloseEvent *event) {
    bool cc = true;
    emit this->canClose(&cc);
    if (cc) {
        event->accept();
    } else {
        event->ignore();
    }
}
