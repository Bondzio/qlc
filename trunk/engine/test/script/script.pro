include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = script_test

QT      += testlib xml

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../mastertimer
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += script_test.cpp ../mastertimer/mastertimer_stub.cpp
HEADERS += script_test.h ../mastertimer/mastertimer_stub.h
