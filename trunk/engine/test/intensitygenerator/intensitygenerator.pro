include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = intensitygenerator_test

QT      += testlib xml
CONFIG  -= app_bundle

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += intensitygenerator_test.cpp
HEADERS += intensitygenerator_test.h
