/*
  Q Light Controller
  hideventdevice.cpp

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

#include <linux/input.h>
#include <errno.h>

#include <QObject>
#include <QString>
#include <QDebug>
#include <QFile>

#include "hideventdevice.h"
#include "hidinput.h"

/**
 * This macro is used to tell if "bit" is set in "array"
 * it selects a byte from the array, and does a boolean AND 
 * operation with a byte that only has the relevant bit set. 
 * eg. to check for the 12th bit, we do (array[1] & 1<<4)
 */
#define test_bit(bit, array)    (array[bit / 8] & (1 << (bit % 8)))

HIDEventDevice::HIDEventDevice(HIDInput* parent, const QString& path) 
	: HIDDevice(parent, path)
{
	m_refCount = 0;
}

HIDEventDevice::~HIDEventDevice()
{
	close();
}

/*****************************************************************************
 * File operations
 *****************************************************************************/

bool HIDEventDevice::open()
{
	bool result = false;

	/* Count the number of times open() has been called so that the devices
	   are opened only once. This is basically reference counting. */
	m_refCount++;
	if (m_refCount > 1)
		return true;

	qWarning() << "*******************************************************";
	qWarning() << "Device file: " << m_file.fileName();

	result = m_file.open(QIODevice::Unbuffered | QIODevice::ReadWrite);
	if (result == false)
	{
		qWarning() << "Unable to open" << m_file.fileName()
			   << "in Read/Write mode:" << m_file.errorString();
		
		result = m_file.open(QIODevice::Unbuffered | QIODevice::ReadOnly);
		if (result == false)
		{
			qWarning() << "Unable to open" << m_file.fileName()
				   << "in Read Only mode:"
				   << m_file.errorString();
		}
	}

	if (result == true)
	{
		/* An event device has been opened in RW or RO mode now.
		   Either way, these should be available. */

		/* Device name */
		char name[128] = "Unknown";
		if (ioctl(m_file.handle(), EVIOCGNAME(sizeof(name)), name) <= 0)
		{
			m_name = QString(strerror(errno));
			perror("ioctl EVIOCGNAME");
		}
		else
		{
			m_name = QString(name);
			qDebug() << "Device name:" << m_name;
		}

		/* Device info */
		if (ioctl(m_file.handle(), EVIOCGID, &m_deviceInfo))
			perror("ioctl EVIOCGID");
		
		/* Supported event types */
		if (ioctl(m_file.handle(), EVIOCGBIT(0, sizeof(m_eventTypes)),
			  m_eventTypes) <= 0)
			perror("ioctl EVIOCGBIT");
		else
			getCapabilities();
	}

	return result;
}

void HIDEventDevice::close()
{
	/* Count the number of times close() has been called so that the devices
	   are closed only after the last user closes this plugin. This is
	   basically reference counting. */
	m_refCount--;
	if (m_refCount > 0)
		return;
	Q_ASSERT(m_refCount == 0);

	while (m_channels.isEmpty() == false)
		delete m_channels.takeFirst();

	m_file.close();
}

QString HIDEventDevice::path() const
{
	return m_file.fileName();
}

t_input_channel HIDEventDevice::channels()
{
	return m_channels.count();
}

void HIDEventDevice::getCapabilities()
{
	QString s;

	qDebug() << "Supported event types:";

	for (int i = 0; i < EV_MAX; i++)
	{
		if (test_bit(i, m_eventTypes))
		{
			switch (i)
			{
			case EV_KEY:
				qDebug() << "\tKeys or Buttons";
				break;

			case EV_ABS:
				qDebug() << "\tAbsolute Axes";
				getAbsoluteAxesCapabilities();
				break;

			case EV_LED:
				qDebug() << "\tLEDs";
				break;

			case EV_REP:
				qDebug() << "\tRepeat";
				break;

			default:
				qDebug() << "\tUnknown event type: " << i;
			}
		}
	}
}

void HIDEventDevice::getAbsoluteAxesCapabilities()
{
	uint8_t mask[ABS_MAX/8 + 1];
	struct input_absinfo feats;
	int r;
	
	memset(mask, 0, sizeof(mask));
	r = ioctl(m_file.handle(), EVIOCGBIT(EV_ABS, sizeof(mask)), mask);
	if (r < 0)
	{
		perror("evdev ioctl");
		return;
	}

	for (int i = 0; i < ABS_MAX; i++)
	{
		if (test_bit(i, mask) != 0)
		{
			r = ioctl(m_file.handle(), EVIOCGABS(i), &feats);
			if (r != 0)
			{
				perror("evdev EVIOCGABS ioctl");
			}
			else
			{
				HIDEventDeviceChannel* channel;
				channel = new HIDEventDeviceChannel(i, EV_ABS,
								 feats.minimum,
								 feats.maximum);
				m_channels.append(channel);

				qDebug() << "\t\tChannel:" << i
					 << "min:" << feats.minimum
					 << "max:" << feats.maximum
					 << "flatness:" << feats.flat
					 << "fuzz:" << feats.fuzz;
			}
		}
	}
}

void HIDEventDevice::readEvent()
{
	struct input_event ev;

	Q_ASSERT(m_file.isOpen() == true);

	if (read(m_file.handle(), &ev, sizeof(struct input_event)) > 0)
	{
		printf("Event: time %ld.%06ld, type %d, code %d, value %d\n",
		       ev.time.tv_sec, ev.time.tv_usec, ev.type,
		       ev.code, ev.value);
	}
}

/*****************************************************************************
 * Enabled status
 *****************************************************************************/

bool HIDEventDevice::isEnabled()
{
	return m_enabled;
}

void HIDEventDevice::setEnabled(bool state)
{
	Q_ASSERT(parent() != NULL);
/*
	if (state == true)
		qobject_cast <HIDInput*> (parent())->addPollDevice(this);
	else
		qobject_cast <HIDInput*> (parent())->removePollDevice(this);
*/
}

/*****************************************************************************
 * Device info
 *****************************************************************************/

QString HIDEventDevice::infoText()
{
	QString info;
	QString str;

	info += QString("<TR>");

	/* File name */
	info += QString("<TD>");
	info += m_file.fileName();
	info += QString("</TD>");

	if (m_file.isOpen() == true)
	{
		/* Name */
		info += QString("<TD>");
		info += m_name;
		info += QString("</TD>");

		/* Mode */
		info += QString("<TD>");
		if (m_file.isReadable() == true)
			info += QString("Read");
		if (m_file.isWritable() == true)
			info += QString("/Write");
		else
			info += QString(" Only");
		info += QString("</TD>");
	}
	else
	{
		/* File is not open, put an error here. */
		info += QString("<TD>");
		info += m_file.errorString();
		info += QString("</TD>");
		info += QString("<TD>");
		info += m_file.errorString();
		info += QString("</TD>");
	}

	info += QString("</TR>");

	return info;
}

/*****************************************************************************
 * Input data
 *****************************************************************************/

void HIDEventDevice::feedBack(t_input_channel /*channel*/,
			      t_input_value /*value*/)
{
}

