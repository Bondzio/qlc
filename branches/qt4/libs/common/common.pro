TEMPLATE 	= lib
LANGUAGE 	= C++
TARGET 		= qlccommon

CONFIG 		+= qt warn_on release build_all
QT 		+= xml
win32:DEFINES 	+= QLC_EXPORT

target.path 	= /usr/lib
INSTALLS 	+= target

# Sources
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
