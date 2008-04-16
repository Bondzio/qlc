/*
  Q Light Controller
  hidinput.cpp

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

#include <QApplication>
#include <QMessageBox>
#include <QStringList>
#include <QDir>

#include "hidinput.h"
#include "hideventdevice.h"
#include "configurehidinput.h"

extern "C" QLCInPlugin* create()
{
	return new HIDInput;
}

/*****************************************************************************
 * Initialization
 *****************************************************************************/

HIDInput::HIDInput() : QLCInPlugin()
{
	m_version = 0x00010000;
	m_name = QString("HID Input");
	m_type = Input;

	open();
}

HIDInput::~HIDInput()
{
}

/*****************************************************************************
 * Open/close
 *****************************************************************************/

int HIDInput::open()
{
	HIDDevice* hidDevice = NULL;
	QDir dir("/dev/input/");
	QStringList nameFilters;
	QStringList entries;
	QStringList::iterator it;
	QString path;

	close();

	nameFilters << "event*";
	entries = dir.entryList(nameFilters, QDir::Files | QDir::System);
	for (it = entries.begin(); it != entries.end(); ++it)
	{
		path = dir.absolutePath() + QDir::separator() + *it;
		hidDevice = device(path);
		if (hidDevice == NULL)
	{
			hidDevice = new HIDEventDevice(this, path);
			hidDevice->open();
			m_devices.append(hidDevice);
		}
	}
}

int HIDInput::close()
{
	HIDDevice* hidDevice = NULL;

	while ((hidDevice = m_devices.takeFirst()) != NULL)
	{
		hidDevice->close();
		delete hidDevice;
	}
}

HIDDevice* HIDInput::device(const QString& path)
{
	QListIterator <HIDDevice*> it(m_devices);

	while (it.hasNext() == true)
	{
		HIDDevice* dev = it.next();
		if (dev->path() == path)
			return dev;
	}

	return NULL;
}

HIDDevice* HIDInput::device(const unsigned int index)
{
	if (index > m_devices.count())
		return NULL;
	else
		return m_devices.at(index);
}

t_input HIDInput::inputs()
{
	return m_devices.count();
}

t_input_channel HIDInput::channels(t_input input)
{
	if (input >= m_devices.count())
		return NULL;
	else
		return m_devices.at(input)->channels();
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

int HIDInput::configure(QWidget* parentWidget)
{
	ConfigureHIDInput conf(parentWidget, this);
	conf.exec();
}

/*****************************************************************************
 * Status
 *****************************************************************************/

QString HIDInput::infoText()
{
	HIDDevice* device = NULL;
	QString info = QString::null;
	QString t;

	/* HTML Title */
	info += QString("<HTML>");
	info += QString("<HEAD>");
	info += QString("<TITLE>Plugin Info</TITLE>");
	info += QString("</HEAD>");
	info += QString("<BODY>");

	/* Plugin title */
	info += QString("<TABLE COLS=\"1\" WIDTH=\"100%\">");
	info += QString("<TR>");
	info += QString("<TD BGCOLOR=\"");
	//info += QApplication::palette().active().highlight().name();
	info += QString("\" COLSPAN=\"3\">");
	info += QString("<FONT COLOR=\"");
	//info += QApplication::palette().active().highlightedText().name();
	info += QString("\" SIZE=\"5\">");
	info += name();
	info += QString("</FONT>");
	info += QString("</TD>");
	info += QString("</TR>");
	info += QString("</TABLE>");

	info += QString("<TABLE COLS=\"3\" WIDTH=\"100%\">");
	info += QString("<TR>");

	/* Device title */
	info += QString("<TD BGCOLOR=\"");
	//info += QApplication::palette().active().highlight().name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	//info += QApplication::palette().active().highlightedText().name();
	info += QString("\" SIZE=\"5\">");
	info += QString("Device");
	info += QString("</FONT>");
	info += QString("</TD>");

	/* Name title */
	info += QString("<TD BGCOLOR=\"");
	//info += QApplication::palette().active().highlight().name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	//info += QApplication::palette().active().highlightedText().name();
	info += QString("\" SIZE=\"5\">");
	info += QString("Name");
	info += QString("</FONT>");
	info += QString("</TD>");

	/* Mode title */
	info += QString("<TD BGCOLOR=\"");
	//info += QApplication::palette().active().highlight().name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	//info += QApplication::palette().active().highlightedText().name();
	info += QString("\" SIZE=\"5\">");
	info += QString("Mode");
	info += QString("</FONT>");
	info += QString("</TD>");
	info += QString("</TR>");

	/* Devices */
	if (m_devices.count() == 0)
	{
		info += QString("<TR>");
		info += QString("<TD COLSPAN=\"3\">");
		info += QString("No HID devices found under /dev/input/");
		info += QString("</TD>");
		info += QString("</TR>");
	}
	else
	{
		QListIterator <HIDDevice*> it(m_devices);
		while (it.hasNext() == true)
			info += it.next()->infoText();
	}

	info += QString("</TABLE>");

	return info;
}

void HIDInput::feedBack(t_input input, t_input_channel channel,
			t_input_value value)
{
}
