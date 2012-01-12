include(../../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = common_test

CONFIG  += qtestlib
QTPLUGIN =

INCLUDEPATH += ../src
DEPENDPATH  += ../src
INCLUDEPATH += ../../../interfaces

HEADERS += midiprotocol_test.h
SOURCES += midiprotocol_test.cpp main.cpp

CONFIG(coverage) {
    LIBS += -L../src -lmidiprotocol
} else {
    HEADERS += ../src/midiprotocol.h
    SOURCES += ../src/midiprotocol.cpp
}
