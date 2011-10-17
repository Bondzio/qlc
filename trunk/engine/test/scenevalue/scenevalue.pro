include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = scenevalue_test

QT      += testlib xml

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += scenevalue_test.cpp
HEADERS += scenevalue_test.h
