include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = rgbmatrix_test

QT      += testlib xml

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../mastertimer
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += rgbmatrix_test.cpp ../mastertimer/mastertimer_stub.cpp
HEADERS += rgbmatrix_test.h ../mastertimer/mastertimer_stub.h
