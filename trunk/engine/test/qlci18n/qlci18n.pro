include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = qlci18n_test

QT      += testlib xml

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += qlci18n_test.cpp
HEADERS += qlci18n_test.h
