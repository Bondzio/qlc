include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = fadechannel_test

QT      += testlib

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += fadechannel_test.cpp
HEADERS += fadechannel_test.h
