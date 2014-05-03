#-------------------------------------------------
#
# Project created by QtCreator 2014-04-25T02:16:37
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gtris
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

SOURCES +=\
        mainwindow.cc \
    main.cc \
    BrickViewer.cc \
    TetrisGameProcess.cc

HEADERS  += mainwindow.h \
    BrickViewer.h \
    TetrisGameProcess.h

FORMS    += mainwindow.ui
