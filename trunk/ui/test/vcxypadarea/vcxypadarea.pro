include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = vcxypadarea_test

QT      += testlib xml gui

INCLUDEPATH += ../../../plugins/interfaces
INCLUDEPATH += ../../../engine/src
INCLUDEPATH += ../../src
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../../engine/src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine -lqlcui

SOURCES += vcxypadarea_test.cpp
HEADERS += vcxypadarea_test.h
