#-------------------------------------------------
#
# Project created by QtCreator 2017-02-24T10:42:53
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = XewlStopwatch
TEMPLATE = app

# hidapi dependents
LIBS += -lcomdlg32 -lsetupapi -lhid

SOURCES += main.cpp\
MainWindow.cpp \
hidapi/hid.c

HEADERS  += MainWindow.h \
hidapi/hidapi.h \
ScopeExit.h \
usbProtocol.h

FORMS    += MainWindow.ui

RESOURCES += \
res/resources.qrc

Release:DESTDIR = ../bin

Release:OBJECTS_DIR = release/.obj
Release:MOC_DIR = release/.moc
Release:RCC_DIR = release/.rcc
Release:UI_DIR = release/.ui
Release:PRECOMPILED_DIR = release

Debug:DESTDIR = ../bin/bin_dbg

Debug:OBJECTS_DIR = debug/.obj
Debug:MOC_DIR = debug/.moc
Debug:RCC_DIR = debug/.rcc
Debug:UI_DIR = debug/.ui
Debug:PRECOMPILED_DIR = debug
