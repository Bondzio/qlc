include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = chaser_test

QT      += testlib xml

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../mastertimer
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += chaser_test.cpp ../mastertimer/mastertimer_stub.cpp
HEADERS += chaser_test.h ../mastertimer/mastertimer_stub.h
