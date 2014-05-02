#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QHBoxLayout>
#include <QDebug>

const QSize playFieldSize(12, 23);
const QSize nextFieldSize(5, 5);
const unsigned initialBrickSize = 20;

const QSize combinedSize(playFieldSize.width() + nextFieldSize.width(), std::max(playFieldSize.height(), nextFieldSize.height()));

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QHBoxLayout *layout = new QHBoxLayout(ui->centralWidget);
    layout->setSpacing(1);
    layout->setMargin(1);
    layout->addWidget(m_playField = new BrickViewer(playFieldSize.width(), playFieldSize.height(), initialBrickSize));
    layout->addWidget(m_nextField = new BrickViewer(nextFieldSize.width(), nextFieldSize.height(), initialBrickSize));
    ui->centralWidget->setLayout(layout);
    adjustSize();
}


BrickViewer *MainWindow::getPlayField() {
    return m_playField;
}


MainWindow::~MainWindow()
{
    delete m_playField;
    delete ui;
}


void MainWindow::resizeEvent(QResizeEvent *event) {
    int w = ui->centralWidget->width();
    int h = ui->centralWidget->height();
    int newBS = std::max(3, std::min(w / combinedSize.width(), h / combinedSize.height()));
    qDebug() << w << " " << h << " " << newBS << endl;
    m_playField->SetBrickSize(newBS);
    m_nextField->SetBrickSize(newBS);
}
