TEMPLATE 	= app
LANGUAGE 	= C++
TARGET 		= qlc-fixtureeditor

CONFIG 		+= qt warn_on release build_all
win32:CONFIG	+= windows
QT 		+= xml

INCLUDEPATH 	+= . ../libs/
LIBS 		+= -L../libs/common/release -lqlccommon

target.path 	= /usr/bin
INSTALLS 	+= target

# Sources
RESOURCES += fixtureeditor.qrc

HEADERS += aboutbox.h \
           app.h \
           editcapability.h \
           editchannel.h \
           editmode.h \
           fixtureeditor.h

FORMS += editcapability.ui \
	 editchannel.ui \
	 editmode.ui \
	 fixtureeditor.ui

SOURCES += aboutbox.cpp \
           app.cpp \
           editcapability.cpp \
           editchannel.cpp \
           editmode.cpp \
           fixtureeditor.cpp \
           main.cpp
