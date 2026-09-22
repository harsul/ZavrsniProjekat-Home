#-------------------------------------------------
#
# Project created by QtCreator 2026-09-22T20:08:40
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ZavrsniProjekat-Home
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
        chartwidget.cpp \
        database.cpp

HEADERS  += mainwindow.h \
        transaction.h \
        chartwidget.h \
        database.h

FORMS    += mainwindow.ui
