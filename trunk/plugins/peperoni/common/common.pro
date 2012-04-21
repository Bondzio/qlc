TEMPLATE = app
LANGUAGE = C++
TARGET   = common

CONFIG    -= app_bundle
CONFIG    += link_pkgconfig
PKGCONFIG += libusb-1.0

HEADERS += ioenumerator.h \
           iodevice.h \
           outputdevice.h \
           inputdevice.h \
           unixioenumerator.h \
           unixpeperonidevice.h

SOURCES += ioenumerator.cpp \
           iodevice.cpp \
           outputdevice.cpp \
           inputdevice.cpp \
           unixioenumerator.cpp \
           unixpeperonidevice.cpp \
           main.cpp
