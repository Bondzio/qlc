include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = qlcpoint_test

QT      += testlib xml
CONFIG  -= app_bundle

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += qlcpoint_test.cpp
HEADERS += qlcpoint_test.h
