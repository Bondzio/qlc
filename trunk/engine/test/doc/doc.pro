include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = doc_test

QT      += testlib xml

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += doc_test.cpp
HEADERS += doc_test.h
