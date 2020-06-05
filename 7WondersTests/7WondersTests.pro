#-------------------------------------------------
#
# Project created by QtCreator 2020-05-29T10:12:17
#
#-------------------------------------------------

QT       += testlib
QT       += core widgets
QT       -= gui

TARGET = tst_tests
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11

TEMPLATE = app


SOURCES += tst_tests.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../7WondersCommon/release/ -l7WondersCommon
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../7WondersCommon/debug/ -l7WondersCommon
else:unix: LIBS += -L$$OUT_PWD/../7WondersCommon/ -l7WondersCommon

INCLUDEPATH += $$PWD/../7WondersCommon
INCLUDEPATH += $$PWD/../7WondersUI
INCLUDEPATH += $$PWD/../7WondersServer
DEPENDPATH += $$PWD/../7WondersCommon
DEPENDPATH += $$PWD/../7WondersUI
DEPENDPATH += $$PWD/../7WondersServer
