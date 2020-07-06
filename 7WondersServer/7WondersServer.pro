QT += core network
QT -= gui

TARGET = 7WondersServer
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11

TEMPLATE = app
#TEMPLATE = lib
#win32:CONFIG += staticlib

SOURCES += main.cpp \
    board.cpp \
    player.cpp \
    human.cpp \
    ai.cpp \
    tcpserver.cpp

HEADERS += \
    board.hpp \
    player.hpp \
    human.hpp \
    ai.hpp \
    tcpserver.hpp


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../7WondersCommon/release/ -l7WondersCommon
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../7WondersCommon/debug/ -l7WondersCommon
else:unix: LIBS += -L$$OUT_PWD/../7WondersCommon/ -l7WondersCommon

INCLUDEPATH += $$PWD/../7WondersCommon
DEPENDPATH += $$PWD/../7WondersCommon
