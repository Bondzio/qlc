lib_LTLIBRARIES = libusbdmxout.la

INCLUDES  = @QT_CFLAGS@ -I$(top_srcdir)/libs
INCLUDES += -DFIXTURES=\"$(datadir)/$(FIXTURES)\"
INCLUDES += -DPIXMAPS=\"$(datadir)/$(PIXMAPS)\"
INCLUDES += -DDOCUMENTS=\"$(datadir)/$(DOCUMENTS)/$(PACKAGE)\"
INCLUDES += -DPLUGINS=\"$(PLUGINS)\"

libusbdmxout_la_LIBADD = @QT_LIBS@

noinst_HEADERS = \
	configureusbdmxout.h \
	usbdmxout.h

libusbdmxout_la_SOURCES = \
	ui_configureusbdmxout.cpp \
	\
	usbdmxout.cpp \
	configureusbdmxout.cpp \
	\
	moc_usbdmxout.cpp \
	moc_configureusbdmxout.cpp

libdir = $(PLUGINS)

# MOC headers
moc_%.cpp: %.h
	$(MOC) $< -o $@

# UIC headers
ui_%.cpp: %.ui
	$(UIC) $< -o $@
