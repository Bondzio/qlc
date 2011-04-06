TEMPLATE = subdirs

# Output plugins
SUBDIRS              += enttecdmxusbout
SUBDIRS              += peperoniout
SUBDIRS              += udmxout
SUBDIRS              += midiout
#unix:SUBDIRS         += olaout
!macx:!win32:SUBDIRS += dmx4linuxout
SUBDIRS              += vellemanout

# Input plugins
SUBDIRS              += ewinginput
SUBDIRS              += midiinput
!macx:!win32:SUBDIRS += hidinput
