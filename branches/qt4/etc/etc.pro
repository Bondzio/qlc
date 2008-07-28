TEMPLATE	= subdirs
CONFIG		=
QT		=
TARGET		= icons

desktop.path	= /usr/share/applications/
desktop.files	+= qlc.desktop \
		   qlc-fixtureeditor.desktop

icons.path	= /usr/share/pixmaps/
icons.files	+= ../gfx/qlc.png \
		   ../gfx/qlc-fixtureeditor.png

INSTALLS	+= icons desktop
