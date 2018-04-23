#-------------------------------------------------
#
# Project created by QtCreator 2016-04-28T19:32:21
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UI
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    rulesmanager.cpp \
    addruledialog.cpp

HEADERS  += mainwindow.h \
    rulesmanager.h \
    addruledialog.h

FORMS    += \
    mainwindow.ui \
    addruledialog.ui
