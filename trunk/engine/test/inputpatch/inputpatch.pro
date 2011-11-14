include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = inputpatch_test

QT      += testlib xml
CONFIG  -= app_bundle

DEPENDPATH   += ../../src
INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
INCLUDEPATH  += ../inputpluginstub
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += inputpatch_test.cpp
HEADERS += inputpatch_test.h
