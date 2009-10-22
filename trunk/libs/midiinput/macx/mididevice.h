/*
  Q Light Controller
  mididevice.h

  Copyright (c) Heikki Junnila

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

#ifndef MIDIDEVICE_H
#define MIDIDEVICE_H

#include <CoreFoundation/CoreFoundation.h>
#include <CoreMIDI/CoreMIDI.h>
#include <QObject>

#include "common/qlctypes.h"

class MIDIDevice;
class MIDIInput;
class QEvent;

/*****************************************************************************
 * MIDIDevice
 *****************************************************************************/

class MIDIDevice : public QObject
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	MIDIDevice(MIDIInput* parent, MIDIEntityRef entity);
	virtual ~MIDIDevice();

	/** Get the device's unique ID and store it to m_uid */
	bool extractUID();

	/** Get the device's name and store it to m_name */
	bool extractName();

	/*********************************************************************
	 * File operations
	 *********************************************************************/
public:
	/** Open the device for input data */
	bool open();

	/** Close the input device */
	void close();

protected:
	/** Reference to the MIDI entity that owns source and destination */
	MIDIEntityRef m_entity;

	/** Reference to the opened MIDI source */
	MIDIEndpointRef m_source;

	/** Reference to the opened MIDI destination */
	MIDIEndpointRef m_destination;

	/** Reference to a virtual port that receives data from the source */
	MIDIPortRef m_inPort;

	/** Reference to a virtual port that sends data to the destination */
	MIDIPortRef m_outPort;

	/*********************************************************************
	 * Device info
	 *********************************************************************/
public:
	QString name() const { return m_name; }
	SInt32 uid() const { return m_uid; }

protected:
	QString m_name;
	SInt32 m_uid;

	/*********************************************************************
	 * Input data
	 *********************************************************************/
protected:
	void customEvent(QEvent* event);

signals:
	/** Signal that is emitted when an input channel's value is changed */
	void valueChanged(MIDIDevice* device, t_input_channel channel,
			  t_input_value value);

	/*********************************************************************
	 * Feedback
	 *********************************************************************/
public:
	void feedBack(t_input_channel channel, t_input_value value);
};

#endif