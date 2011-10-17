include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = qlcfile_test

QT      += testlib xml

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += qlcfile_test.cpp
HEADERS += qlcfile_test.h
