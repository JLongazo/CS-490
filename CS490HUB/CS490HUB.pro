#-------------------------------------------------
#
# Project created by QtCreator 2014-01-24T16:15:53
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CS490HUB
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    objective.cpp \
    taskallocator.cpp

HEADERS  += mainwindow.h \
    objective.h \
    taskallocator.h \
    readerthread.h

FORMS    += mainwindow.ui
