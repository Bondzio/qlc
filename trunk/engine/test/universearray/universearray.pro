include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = universearray_test

QT      += testlib xml
CONFIG  -= app_bundle

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += universearray_test.cpp
HEADERS += universearray_test.h
