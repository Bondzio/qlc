/*
  Q Light Controller
  llaout.cpp
  
  Copyright (c) Simon Newton
                Heikki Junnila
  
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

#include <linux/errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include <QApplication>
#include <QString>
#include <QMutex>
#include <QFile>

#include <lla/LlaClient.h>

#include "common/qlcfile.h"
#include "configurellaout.h"
#include "llaout.h"

static QMutex _mutex;

/*****************************************************************************
 * Name
 *****************************************************************************/

QString LlaOut::name()
{
	return QString("LLA Output");
}

/*****************************************************************************
 * Open/close
 *****************************************************************************/

int LlaOut::open()
{
	for (t_channel i = 0; i < KChannelMax; i++)
		m_values[i] = 0;

	m_lla = new LlaClient();
	
	if (m_lla == NULL)
		return -1;
	
	if (m_lla->start() < 0)
	{
		delete m_lla;
		m_lla = NULL;
		return -1;
	}

	return 0;
}

int LlaOut::close()
{
	if (m_lla != NULL)
	{
		m_lla->stop();
		delete m_lla;
		m_lla = NULL;
	}
	
	return 0;
}

int LlaOut::outputs()
{
	return 1;
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

int LlaOut::configure()
{
	ConfigureLlaOut conf(NULL, this);
	return conf.exec();
}

/*****************************************************************************
 * Status
 *****************************************************************************/

QString LlaOut::infoText()
{
	QString t;
	QString str;

	str += QString("<HTML>");
	str += QString("<HEAD>");
	str += QString("<TITLE>Plugin Info</TITLE>");
	str += QString("</HEAD>");
	str += QString("<BODY>");

	/* Title */
	str += QString("<TABLE COLS=\"1\" WIDTH=\"100%\">");
	str += QString("<TR>");
	str += QString("<TD BGCOLOR=\"");
	str += QApplication::palette().color(QPalette::Highlight).name();
	str += QString("\">");
	str += QString("<FONT COLOR=\"");
	str += QApplication::palette().color(QPalette::HighlightedText).name();
	str += QString("\" SIZE=\"5\">");
	str += name();
	str += QString("</FONT>");
	str += QString("</TD>");
	str += QString("</TR>");
	str += QString("</TABLE>");

	str += QString("</BODY>");
	str += QString("</HTML>");
	
	return str;
}

/*****************************************************************************
 * Value read/write
 *****************************************************************************/

int LlaOut::writeChannel(t_channel channel, t_value value)
{
	int r = 0;

	_mutex.lock();
	
	m_values[channel] = value;
	
	//which interface should we write to?
	int uniNo = int(channel / 512);
	
	if (m_lla != NULL)
	{
		m_lla->send_dmx(uniNo + 1, &m_values[uniNo * 512], 512);
		m_lla->fd_action(0);
	}
	
	_mutex.unlock();
	
	return r;
}

int LlaOut::writeRange(t_channel address, t_value* values, t_channel num)
{
	int r = 0;
	QString txt;
	
	Q_ASSERT(values != NULL);

	_mutex.lock();
	
	// which one is the first universe to write to?
	int uni = int(address / 512);
	
	// how many universes?
	int lastUni = (address + num) / 512;
	
	memcpy(m_values + address, values, num * sizeof(t_value));
	
	if (m_lla != NULL)
	{
		for(int i = uni; i <= lastUni; i++)
			m_lla->send_dmx(i + 1, &m_values[i * 512], 512);
	}
	
	m_lla->fd_action(0);
	_mutex.unlock();
	
	return r;
}

int LlaOut::readChannel(t_channel channel, t_value &value)
{
	_mutex.lock();
	value = m_values[channel];
	_mutex.unlock();
	
	return 0;
}

int LlaOut::readRange(t_channel address, t_value* values, t_channel num)
{
	Q_ASSERT(values != NULL);
	
	_mutex.lock();
	memcpy(values, m_values + address, num * sizeof(t_value));
	_mutex.unlock();
	
	return 0;
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(llaout, LlaOut)
