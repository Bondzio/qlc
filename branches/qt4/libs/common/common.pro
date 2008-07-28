TEMPLATE 	= lib
LANGUAGE 	= C++
TARGET 		= qlccommon

CONFIG 		+= qt warn_on
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
           qlcphysical.h \
           qlctypes.h
	   
SOURCES += qlccapability.cpp \
           qlcchannel.cpp \
           qlcdocbrowser.cpp \
           qlcfile.cpp \
           qlcfixturedef.cpp \
           qlcfixturemode.cpp \
           qlcphysical.cpp
