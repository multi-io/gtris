#-------------------------------------------------
#
# Project created by QtCreator 2014-04-25T02:16:37
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gtris
TEMPLATE = app


SOURCES +=\
        mainwindow.cc \
    main.cc \
    BrickViewer.cc

HEADERS  += mainwindow.h \
    BrickViewer.h

FORMS    += mainwindow.ui
