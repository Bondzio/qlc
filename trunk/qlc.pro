include(variables.pri)

TEMPLATE = subdirs

SUBDIRS      += engine
SUBDIRS      += ui
SUBDIRS      += main
SUBDIRS      += fixtures
SUBDIRS      += inputprofiles
SUBDIRS      += fixtureeditor
SUBDIRS      += etc
SUBDIRS      += docs
macx:SUBDIRS += launcher
SUBDIRS      += plugins

unix:!macx:DEBIAN_CLEAN    += debian/*.substvars debian/*.log debian/*.debhelper
unix:!macx:DEBIAN_CLEAN    += debian/files debian/dirs
unix:!macx:QMAKE_CLEAN     += $$DEBIAN_CLEAN
unix:!macx:QMAKE_DISTCLEAN += $$DEBIAN_CLEAN

# Unit testing thru "make check"
unittests.target = check
QMAKE_EXTRA_TARGETS += unittests
unix:unittests.commands += ./unittest.sh
win32:unittests.commands += unittest.bat

# Unit test coverage measurement
coverage.target = lcov
QMAKE_EXTRA_TARGETS += coverage
unix:coverage.commands += ./coverage.sh
win32:coverage.commands = @echo Get a better OS.

# Leave this on the last row of this file
macx:SUBDIRS += macx
