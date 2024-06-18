#-------------------------------------------------
#
# Project created by QtCreator 2022-04-23T14:07:08
#
#-------------------------------------------------
QT       += charts
QT       += core gui
CONFIG += utf8
QT       += core gui multimedia charts
QT       += core gui network
QT += multimedia
QT += multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Sender
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    audiolevel.cpp \
        main.cpp \
        mainwindow.cpp \
    modulation.cpp \
    sender.cpp

HEADERS += \
    audiolevel.h \
        mainwindow.h \
    modulation.h \
    sender.h

FORMS += \
        mainwindow.ui
