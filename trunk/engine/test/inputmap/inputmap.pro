include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = inputmap_test

QT      += testlib xml
CONFIG  -= app_bundle

DEPENDPATH   += ../../src
INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
INCLUDEPATH  += ../inputpluginstub
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += inputmap_test.cpp
HEADERS += inputmap_test.h
