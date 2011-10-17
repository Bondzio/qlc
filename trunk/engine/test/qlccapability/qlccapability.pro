include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = qlccapability_test

QT      += testlib xml

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += qlccapability_test.cpp
HEADERS += qlccapability_test.h
