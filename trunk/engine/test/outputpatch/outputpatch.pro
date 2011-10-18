include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = outputpatch_test

QT      += testlib xml
CONFIG  -= app_bundle

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
INCLUDEPATH  += ../outputpluginstub
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += outputpatch_test.cpp
HEADERS += outputpatch_test.h
