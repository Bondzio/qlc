/*
  Q Light Controller
  midiinput.h

  Copyright (C) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef MIDIINPUT_H
#define MIDIINPUT_H

#include <CoreFoundation/CoreFoundation.h>
#include <CoreMIDI/CoreMIDI.h>
#include <QStringList>
#include <QString>
#include <QList>

#include "qlcinplugin.h"
#include "qlctypes.h"

class MIDIDevice;
class MIDIInput;

/*****************************************************************************
 * MIDIInput
 *****************************************************************************/

class MIDIInput : public QObject, public QLCInPlugin
{
	Q_OBJECT
	Q_INTERFACES(QLCInPlugin)

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/** \reimp */
	void init();

	/** \reimp */
	virtual ~MIDIInput();

	/** \reimp */
	void open(quint32 input = 0);

	/** \reimp */
	void close(quint32 input = 0);

	const MIDIClientRef client() const { return m_client; }

protected:
	MIDIClientRef m_client;

	/*********************************************************************
	 * Devices
	 *********************************************************************/
public:
	void rescanDevices();

	MIDIDevice* deviceByUID(SInt32 uid);
	MIDIDevice* device(quint32 input);

	void addDevice(MIDIDevice* device);
	void removeDevice(MIDIDevice* device);

	const QList <MIDIDevice*>& devices() const { return m_devices; }

signals:
	void deviceAdded(MIDIDevice* device);
	void deviceRemoved(MIDIDevice* device);
	void configurationChanged();

protected:
	QList <MIDIDevice*> m_devices;

	/*********************************************************************
	 * Name
	 *********************************************************************/
public:
	/** \reimp */
	QString name();

	/*********************************************************************
	 * Inputs
	 *********************************************************************/
public:
	/** \reimp */
	QStringList inputs();

	/*********************************************************************
	 * Configuration
	 *********************************************************************/
public:
	/** \reimp */
	void configure();

	/*********************************************************************
	 * Status
	 *********************************************************************/
public:
	/** \reimp */
	QString infoText(quint32 input = KInputInvalid);

	/*********************************************************************
	 * Input data
	 *********************************************************************/
protected slots:
	/** Receives MIDI input data from MIDIDevices */
	void slotDeviceValueChanged(MIDIDevice* device, quint32 channel, uchar value);

signals:
	/** \reimp */
	void valueChanged(QLCInPlugin* plugin, quint32 line, quint32 channel,
			  uchar value);

public:
	/** \reimp */
	void connectInputData(QObject* listener);

	/** \reimp */
	void feedBack(quint32 input, quint32 channel, uchar value);
};

#endif
