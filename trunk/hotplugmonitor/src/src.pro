include(../../variables.pri)
TEMPLATE = lib
LANGUAGE = C++
TARGET   = hotplugmonitor

unix:!macx: {
    SOURCES   += hpmprivate-udev.cpp
    HEADERS   += hpmprivate-udev.h
    CONFIG    += link_pkgconfig
    PKGCONFIG += libudev
}

macx: {
    SOURCES   += hpmprivate-iokit.cpp
    HEADERS   += hpmprivate-iokit.h
    LIBS      += -framework IOKit -framework CoreFoundation
}

win32: {
    SOURCES += hpmprivate-win32.cpp
    HEADERS += hpmprivate-win32.h
}

SOURCES += hotplugmonitor.cpp
HEADERS += hotplugmonitor.h
