/*
  Q Light Controller
  hidinput.h

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

#ifndef HIDINPUT_H
#define HIDINPUT_H

#include <QList>

#include "common/qlcinplugin.h"
#include "common/qlctypes.h"

#include "hiddevice.h"
#include "hidpoller.h"

/**
 * This lib's only exported function that is used to create instances of
 * class HIDInput
 */
extern "C" QLCInPlugin* create();

/*****************************************************************************
 * HIDInput
 *****************************************************************************/

class HIDInput : public QLCInPlugin
{
	Q_OBJECT

	friend class ConfigureHIDInput;
	friend class HIDPoller;

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	HIDInput();
	virtual ~HIDInput();

	/*********************************************************************
	 * Open/close
	 *********************************************************************/
public:
	virtual int open();
	virtual int close();

protected:
	HIDDevice* device(const QString& path);
	HIDDevice* device(const unsigned int index);

protected:
	QList <HIDDevice*> m_devices;

	/*********************************************************************
	 * Inputs & channels
	 *********************************************************************/
public:
	virtual t_input inputs();
	virtual t_input_channel channels(t_input input);

	/*********************************************************************
	 * Configuration
	 *********************************************************************/
public:
	virtual int configure(QWidget* parentWidget);

	/*********************************************************************
	 * Status
	 *********************************************************************/
public:
	virtual QString infoText();

	/*********************************************************************
	 * Polled devices
	 *********************************************************************/
public:
	/**
	 * Add a HID device to be polled for input events
	 */
	bool addPollDevice(HIDDevice* device);

	/**
	 * Remove a HID device from the list of polled devices
	 */
	bool removePollDevice(HIDDevice* device);

protected:
	HIDPoller* m_poller;

	/*********************************************************************
	 * Input data
	 *********************************************************************/
public:
	virtual void feedBack(t_input input, t_input_channel channel,
			      t_input_value value);
};

#endif
