include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = ewing_test

QT     += core gui network testlib

INCLUDEPATH += ../../interfaces
INCLUDEPATH += ../src
LIBS        += -L../src -lewinginput

SOURCES += eplaybackwing_test.cpp \
           eprogramwing_test.cpp \
           eshortcutwing_test.cpp \
           ewing_test.cpp \
           main.cpp

HEADERS += eplaybackwing_test.h \
           eprogramwing_test.h \
           eshortcutwing_test.h \
           ewing_test.h
