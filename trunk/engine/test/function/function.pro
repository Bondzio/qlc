include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = function_test

QT      += testlib xml

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += function_test.cpp function_stub.cpp
HEADERS += function_test.h function_stub.h
