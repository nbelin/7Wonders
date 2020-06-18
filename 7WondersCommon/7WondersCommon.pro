#-------------------------------------------------
#
# Project created by QtCreator 2019-03-14T17:56:42
#
#-------------------------------------------------

QT       -= gui

TARGET = 7WondersCommon
TEMPLATE = lib
CONFIG += c++11

win32:CONFIG += staticlib

SOURCES += \
    action.cpp \
    choice.cpp \
    resource.cpp \
    card.cpp \
    wonder.cpp \
    boardview.cpp \
    playerview.cpp \
    resourceproduction.cpp \
    tools.cpp \
    tcpcommon.cpp

HEADERS +=\
    action.hpp \
    choice.hpp \
    resource.hpp \
    card.hpp \
    wonder.hpp \
    tools.hpp \
    boardview.hpp \
    playerview.hpp \
    resourceproduction.hpp \
    tcpcommon.hpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}
