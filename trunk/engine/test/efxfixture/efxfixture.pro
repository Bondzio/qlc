include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = efxfixture_test

QT      += testlib xml
CONFIG  -= app_bundle

DEPENDPATH   += ../../src
INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../mastertimer
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += efxfixture_test.cpp ../mastertimer/mastertimer_stub.cpp
HEADERS += efxfixture_test.h ../mastertimer/mastertimer_stub.h
