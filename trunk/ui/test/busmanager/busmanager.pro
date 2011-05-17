include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = busmanager_test

QT      += testlib gui xml

INCLUDEPATH += ../../src
INCLUDEPATH += ../../../engine/src
DEPENDPATH  += ../../src

QMAKE_LIBDIR += ../../src
QMAKE_LIBDIR += ../../../engine/src
LIBS         += -lqlcui -lqlcengine

# Test sources
SOURCES += busmanager_test.cpp
HEADERS += busmanager_test.h
