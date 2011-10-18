include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = genericfader_test

QT      += testlib
CONFIG  -= app_bundle

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += genericfader_test.cpp
HEADERS += genericfader_test.h
