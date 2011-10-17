include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = qlcinputprofile_test

QT      += testlib xml

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += qlcinputprofile_test.cpp
HEADERS += qlcinputprofile_test.h
