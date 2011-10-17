include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = qlcchannel_test

QT      += testlib xml

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += qlcchannel_test.cpp
HEADERS += qlcchannel_test.h
