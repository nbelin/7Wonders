#-------------------------------------------------
#
# Project created by QtCreator 2019-03-14T17:57:45
#
#-------------------------------------------------

QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = 7WondersUI
TEMPLATE = app
CONFIG += c++11


CONFIG += file_copies
RESOURCES += ../7Wonders.qrc


SOURCES += main.cpp \
    ui.cpp \
    tcpclient.cpp

HEADERS += \
    ui.hpp \
    tcpclient.hpp

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../7WondersCommon/release/ -l7WondersCommon
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../7WondersCommon/debug/ -l7WondersCommon
else:unix: LIBS += -L$$OUT_PWD/../7WondersCommon/ -l7WondersCommon

INCLUDEPATH += $$PWD/../7WondersCommon
DEPENDPATH += $$PWD/../7WondersCommon
