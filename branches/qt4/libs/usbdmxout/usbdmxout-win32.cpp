/*
  Q Light Controller
  usbdmxout-win32.cpp
  
  Copyright (c)	Heikki Junnila
  
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

#include <windows.h>

#include <QApplication>
#include <QPalette>
#include <QDebug>
#include <QString>
#include <QColor>

#include "configureusbdmxout.h"
#include "usbdmxout-win32.h"
#include "usbdmx-dynamic.h"

extern "C" struct usbdmx_functions *usbdmx;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void USBDMXOut::init()
{
	/* Initialize value buffer */
	for (t_channel ch = 0; ch < MAX_USBDMX_DEVICES * 512; ch++)
		m_values[ch] = 0;

	/* Initialize device handles */
	for (int i = 0; i < MAX_USBDMX_DEVICES; i++)
	{
		m_devices[i] = 0;
	}

	m_refCount = 0;
}

/*****************************************************************************
 * Plugin open/close
 *****************************************************************************/

int USBDMXOut::open()
{
	/* Count the number of times open() has been called so that the devices
	   are opened only once. This is basically reference counting. */
	m_refCount++;
	if (m_refCount > 1)
		return 0;

	/* Load usbdmx.dll */
	usbdmx = usbdmx_init();
	if (!usbdmx)
	{
		qWarning() << "Loading USBDMX.DLL failed. Abort.";
		return 1;
	}

	/* verify USBDMX dll version */
	if (!USBDMX_DLL_VERSION_CHECK(usbdmx))
	{
		qWarning() << "USBDMX.DLL version does not match. Abort.";
		qWarning() << "Found" << usbdmx->version << "but expected"
			   << USBDMX_DLL_VERSION;
		return 1;
	}

	printf("Using USBDMX.DLL version 0x%x\n\n", usbdmx->version());

	for (int i = 0; i < MAX_USBDMX_DEVICES; i++)
	{
		HANDLE handle;
		
		/* Open the device */
		if (!usbdmx->open(i, &handle))
		{
			m_devices[i] = 0;
			continue;
		}
		else
		{
			USHORT version;
			m_devices[i] = handle;
	
			/* Identify the interface */
			if (usbdmx->is_xswitch(handle))
				qDebug() << "Found an X-Switch";
			else if (usbdmx->is_rodin1(handle))
				qDebug() << "Found a Rodin1";
			else if (usbdmx->is_rodin2(handle))
				qDebug() << "Found a Rodin2";
			else if (usbdmx->is_rodint(handle))
				qDebug() << "Found a RodinT";
			else if (usbdmx->is_usbdmx21(handle))
				qDebug() << "Found a USBDMX21";
		
			usbdmx->device_version(handle, &version);
			qDebug() << "\tVersion" << version;
		}
	}

	return 0;
}

int USBDMXOut::close()
{
	/* Count the number of times close() has been called so that the devices
	   are closed only after the last user closes this plugin. This is
	   basically reference counting. */
	m_refCount--;
	if (m_refCount > 0)
		return 0;
	Q_ASSERT(m_refCount == 0);

	for (int i = 0; i < MAX_USBDMX_DEVICES; i++)
	{
		/* Close the interface if it exists */
		if (m_devices[i] != 0)
			usbdmx->close(m_devices[i]);
		m_devices[i] = 0;
	}

	return 0;
}

int USBDMXOut::outputs()
{
	return MAX_USBDMX_DEVICES;
}

/*****************************************************************************
 * Name
 *****************************************************************************/

QString USBDMXOut::name()
{
	return QString("USB DMX Output");
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

int USBDMXOut::configure()
{
	int r;

	open();

	ConfigureUSBDMXOut conf(NULL, this);
	r = conf.exec();

	close();

	return r;
}

/*****************************************************************************
 * Plugin status
 *****************************************************************************/

QString USBDMXOut::infoText()
{
	QString info;
	QString s;

	/* HTML page Title */
	info += QString("<HTML>");
	info += QString("<HEAD>");
	info += QString("<TITLE>Plugin Info</TITLE>");
	info += QString("</HEAD>");
	info += QString("<BODY>");

	/* Plugin title */
	info += QString("<TABLE COLS=\"1\" WIDTH=\"100%\">");
	info += QString("<TR>");
	info += QString("<TD BGCOLOR=\"");
	info += QApplication::palette().color(QPalette::Highlight).name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += QApplication::palette().color(QPalette::HighlightedText).name();
	info += QString("\" SIZE=\"5\">");
	info += name();
	info += QString("</FONT>");
	info += QString("</TD>");
	info += QString("</TR>");
	info += QString("</TABLE>");

	/*********************************************************************
	 * Outputs
	 *********************************************************************/

	/* Title */
	info += QString("<TABLE COLS=\"3\" WIDTH=\"100%\">");
	info += QString("<TR>");
	info += QString("<TD BGCOLOR=\"");
	info += QApplication::palette().color(QPalette::Highlight).name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += QApplication::palette().color(QPalette::HighlightedText).name();
	info += QString("\">");
	info += QString("Output");
	info += QString("</FONT>");
	info += QString("</TD>");

	info += QString("<TD BGCOLOR=\"");
	info += QApplication::palette().color(QPalette::Highlight).name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += QApplication::palette().color(QPalette::HighlightedText).name();
	info += QString("\">");
	info += QString("Device name");
	info += QString("</FONT>");
	info += QString("</TD>");
	info += QString("</TR>");

	open();

	/* Output lines */
	for (int i = 0; i < MAX_USBDMX_DEVICES; i++)
	{
		info += QString("<TR>");

		s.sprintf("%d", i + 1);
		info += QString("<TD>" + s + "</TD>");

		if (m_devices[i] != NULL)
		{
			USHORT version;
			usbdmx->device_version(m_devices[i], &version);
			
			if (usbdmx->is_xswitch(m_devices[i]))
				info += QString("<TD>X-Switch V%1</TD>")
					.arg(version);
			else if (usbdmx->is_rodin1(m_devices[i]))
				info += QString("<TD>Rodin1 V%1</TD>")
					.arg(version);
			else if (usbdmx->is_rodin2(m_devices[i]))
				info += QString("<TD>Rodin2 V%1</TD>")
					.arg(version);
			else if (usbdmx->is_rodint(m_devices[i]))
				info += QString("<TD>RodinT V%1</TD>")
					.arg(version);
			else if (usbdmx->is_usbdmx21(m_devices[i]))
				info += QString("<TD>USBDMX21 V%1</TD>")
					.arg(version);
		}
		else
		{
			info += QString("<TD>Nothing</TD>");
		}
		
		info += QString("</TR>");
	}

	close();

	info += QString("</TABLE>");

	info += QString("</BODY>");
	info += QString("</HTML>");

	return info;
}

/*****************************************************************************
 * Value Read/Write
 *****************************************************************************/

int USBDMXOut::writeChannel(t_channel channel, t_value value)
{
	int ifaceNo = int(channel / 512);
	int channelNo = channel % 512;
	int r = 0;
	
	m_mutex.lock();

	m_values[channel] = value;
	
	if (m_devices[ifaceNo] != 0)
	{
		UCHAR status;
		USHORT timestamp;
		
		if (!usbdmx->tx(m_devices[ifaceNo], /* Handle to the interface */
				0, /* Physical universe addressed on Rodin1/2: only 0 is supported on RodinT: 0 tx side, 1 rx side */
				513, /* Number of slots to be transmitted, including startcode */
				m_values + channelNo, /* Buffer with dmx data ([0] is the startcode */
				USBDMX_BULK_CONFIG_BLOCK, /* Configuration for this frame, see usbdmx.h for details */
				100e-3,	/* Parameter for configuration [s], see usbdmx.h for details, in this case: block 100ms with respect to previous frame */
				200e-6,	/* length of break [s]. If 0, no break is generated */
				20e-6,	/* Length of mark-after-break [s], If 0, no MaB is generated */
				&timestamp, /* timestamp of the frame [ms] */
				&status)) /* status information */
		{
			if (!USBDMX_BULK_STATUS_IS_OK(status))
			{
				qWarning() << "ERROR: usbdmx_tx(): status ="
					   << status;
			}
		}
		else
		{
			r = 1;
		}
	}

	m_mutex.unlock();
	
	return r;
}

int USBDMXOut::writeRange(t_channel address, t_value* values, t_channel num)
{
	Q_ASSERT(values != NULL);

	int ifaceNo = int(address / 512);
	int firstChannel = address % 512;
	int r = 0;
	
	m_mutex.lock();

	memcpy(m_values + address, values, num * sizeof(t_value));

	if (m_devices[ifaceNo] != 0)
	{
		/* TODO */
	}
  
	m_mutex.unlock();

	return r;
}

int USBDMXOut::readChannel(t_channel channel, t_value &value)
{
	m_mutex.lock();
	value = m_values[channel];
	m_mutex.unlock();

	return 0;
}

int USBDMXOut::readRange(t_channel address, t_value* values, t_channel num)
{
	Q_ASSERT(values != NULL);

	m_mutex.lock();
	memcpy(values, m_values + address, num * sizeof(t_value));
	m_mutex.unlock();

	return 0;
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(usbdmxout, USBDMXOut)
