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
    taskallocator.cpp \
    robot.cpp \
    keypressfilter.cpp

HEADERS  += mainwindow.h \
    objective.h \
    taskallocator.h \
    robot.h \
    readerthread.h \
    keypressfilter.h

FORMS    += mainwindow.ui
