TEMPLATE = lib
LANGUAGE = C++
CONFIG += qt
TARGET = qlccommon
DEPENDPATH += .
INCLUDEPATH += .
QT += xml
DEFINES += QLC_EXPORT

# Input
HEADERS += qlccapability.h \
           qlcchannel.h \
           qlcdocbrowser.h \
           qlcfile.h \
           qlcfixturedef.h \
           qlcfixturemode.h \
           qlcinplugin.h \
           qlcoutplugin.h \
           qlcphysical.h \
           qlcplugin.h \
           qlctypes.h
	   
SOURCES += qlccapability.cpp \
           qlcchannel.cpp \
           qlcdocbrowser.cpp \
           qlcfile.cpp \
           qlcfixturedef.cpp \
           qlcfixturemode.cpp \
           qlcinplugin.cpp \
           qlcoutplugin.cpp \
           qlcphysical.cpp \
           qlcplugin.cpp
