TEMPLATE = subdirs

macx:SUBDIRS       += macx
unix:!macx:SUBDIRS += alsa
