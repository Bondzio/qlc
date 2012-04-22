TEMPLATE = subdirs

SUBDIRS              += enttecdmxusbout
SUBDIRS              += peperoni
SUBDIRS              += udmx
SUBDIRS              += midi
#unix:SUBDIRS         += olaout
!macx:!win32:SUBDIRS += dmx4linux
!macx:SUBDIRS        += vellemanout
SUBDIRS              += enttecwing
!macx:!win32:SUBDIRS += hid
