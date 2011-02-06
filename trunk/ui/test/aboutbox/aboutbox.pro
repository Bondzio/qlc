include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = aboutbox_test

QT      += testlib

INCLUDEPATH += ../../src
INCLUDEPATH += ../../../engine/src

# Test sources
SOURCES += aboutbox_test.cpp
HEADERS += aboutbox_test.h

# Sources to test
SOURCES += ../../src/aboutbox.cpp
HEADERS += ../../src/aboutbox.h
FORMS += ../../src/aboutbox.ui
