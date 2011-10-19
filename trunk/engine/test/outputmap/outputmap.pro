include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = outputmap_test

QT      += testlib xml
CONFIG  -= app_bundle

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
INCLUDEPATH  += ../outputpluginstub
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += outputmap_test.cpp
HEADERS += outputmap_test.h