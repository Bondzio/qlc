include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = vcdockslider_test

QT      += testlib xml gui

INCLUDEPATH += ../../../plugins/interfaces
INCLUDEPATH += ../../../engine/src
INCLUDEPATH += ../../src
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../../engine/src
QMAKE_LIBDIR += ../../src
LIBS        += -lqlcengine -lqlcui

# Test sources
SOURCES += vcdockslider_test.cpp
HEADERS += vcdockslider_test.h
