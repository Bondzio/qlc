/*
  Q Light Controller
  ewinginput.cpp

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

#include <QCoreApplication>
#include <QMessageBox>
#include <QStringList>
#include <QUdpSocket>
#include <QDebug>

#include "ewinginput.h"
#include "eplaybackwing.h"
#include "eshortcutwing.h"
#include "eprogramwing.h"
#include "ewing.h"

/*****************************************************************************
 * EWingInput Initialization
 *****************************************************************************/

void EWingInput::init()
{
	/* Create a new UDP socket and start listening to packets coming to
	   any local address. */
	m_socket = new QUdpSocket(this);
	m_socket->bind(QHostAddress::Any, EWing::UDPPort);
	connect(m_socket, SIGNAL(readyRead()), this, SLOT(slotReadSocket()));
}

EWingInput::~EWingInput()
{
	while (m_devices.isEmpty() == false)
		delete m_devices.takeFirst();
}

void EWingInput::open(t_input input)
{
	EWing* dev = device(input);
	if (dev == NULL)
		qDebug() << name() << "has no input number:" << input;
}

void EWingInput::close(t_input input)
{
	EWing* dev = device(input);
	if (dev == NULL)
		qDebug() << name() << "has no input number:" << input;
}

void EWingInput::slotReadSocket()
{
	while (m_socket->hasPendingDatagrams() == true)
	{
		QHostAddress sender;
		QByteArray data;
		EWing* wing;

		/* Read data from socket */
		data.resize(m_socket->pendingDatagramSize());
		m_socket->readDatagram(data.data(), data.size(), &sender);

		/* Check, whether we already have a device from this address */
		wing = device(sender);
		if (wing == NULL)
		{
			/* New address. Create a new device. */
			wing = createWing(this, sender, data);
			if (wing != NULL)
				addDevice(wing);
		}

		if (wing != NULL)
			wing->parseData(data);
	}
}

EWing* EWingInput::createWing(QObject* parent, const QHostAddress& address,
				const QByteArray& data)
{
	EWing* ewing;

	/* Check, that the message is from an ENTTEC Wing */
	if (EWing::isOutputData(data) == false)
		return NULL;

	switch (EWing::resolveType(data))
	{
	case EWing::Playback:
		ewing = new EPlaybackWing(parent, address, data);
		break;

	case EWing::Shortcut:
		ewing = new EShortcutWing(parent, address, data);
		break;

	case EWing::Program:
		ewing = new EProgramWing(parent, address, data);
		break;

	default:
		ewing = NULL;
		break;
	}

	return ewing;
}

/*****************************************************************************
 * Devices
 *****************************************************************************/

EWing* EWingInput::device(const QHostAddress& address)
{
	QListIterator <EWing*> it(m_devices);
	while (it.hasNext() == true)
	{
		EWing* dev = it.next();
		if (dev->address() == address)
			return dev;
	}

	return NULL;
}

EWing* EWingInput::device(int index)
{
	if (index > m_devices.count())
		return NULL;
	else
		return m_devices.at(index);
}

static bool ewing_device_sort(const EWing* d1, const EWing* d2)
{
	/* Sort devices based on their addresses. Lexical sorting is enough. */
	return (d1->address().toString() < d2->address().toString());
}

void EWingInput::addDevice(EWing* device)
{
	Q_ASSERT(device != NULL);

	connect(device, SIGNAL(valueChanged(t_input_channel,t_input_value)),
		this, SLOT(slotValueChanged(t_input_channel,t_input_value)));

	m_devices.append(device);

	/* To maintain some persistency with the indices of multiple devices
	   between sessions they need to be sorted according to some 
	   (semi-)permanent criteria. Their addresses shouldn't change too
	   often, so let's use that. */
	qSort(m_devices.begin(), m_devices.end(), ewing_device_sort);
	
	emit deviceAdded(device);
}

void EWingInput::removeDevice(EWing* device)
{
	Q_ASSERT(device != NULL);
	m_devices.removeAll(device);
	emit deviceRemoved(device);
	delete device;
}

/*****************************************************************************
 * Name
 *****************************************************************************/

QString EWingInput::name()
{
	return QString("ENTTEC Wing Input");
}

/*****************************************************************************
 * Inputs
 *****************************************************************************/

QStringList EWingInput::inputs()
{
	QStringList list;

	QListIterator <EWing*> it(m_devices);
	while (it.hasNext() == true)
		list << it.next()->name();

	return list;
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void EWingInput::configure()
{
	QMessageBox::information(NULL, name(),
				tr("This plugin has no configurable options."));
}

/*****************************************************************************
 * Status
 *****************************************************************************/

QString EWingInput::infoText(t_input input)
{
	QString str;

	str += QString("<HTML>");
	str += QString("<HEAD>");
	str += QString("<TITLE>%1</TITLE>").arg(name());
	str += QString("</HEAD>");
	str += QString("<BODY>");

	str += QString("<H3>%1</H3>").arg(name());

	if (input == KInputInvalid)
	{
		/* Plugin or just an invalid input selected. Display generic
		   information. */
		str += QString("<P>This plugin provides input support for ");
		str += QString("ENTTEC Playback and Shortcut Wings.</P>");
		
		if (m_socket->state() != QAbstractSocket::BoundState)
		{
			str += QString("<P>Unable to bind to UDP port %1</P>")
				.arg(EWing::UDPPort);
		}
		else
		{
			str += QString("<P>Listening to UDP port %1</P>")
				.arg(EWing::UDPPort);
		}
	}
	else
	{
		/* A specific input line selected. Display its information if
		   available. */
		EWing* dev = device(input);
		if (dev != NULL)
			str += dev->infoText();
		else
			str += tr("<P>%1: Device not found.</P>").arg(input+1);
	}

	str += QString("</BODY>");
	str += QString("</HTML>");

	return str;
}

/*****************************************************************************
 * Input data
 *****************************************************************************/

void EWingInput::slotValueChanged(t_input_channel channel, t_input_value value)
{
	EWing* wing = qobject_cast<EWing*> (QObject::sender());
	emit valueChanged(this, m_devices.indexOf(wing), channel, value);
}

void EWingInput::connectInputData(QObject* listener)
{
	Q_ASSERT(listener != NULL);

	connect(this, SIGNAL(valueChanged(QLCInPlugin*,t_input,t_input_channel,
					  t_input_value)),
		listener, SLOT(slotValueChanged(QLCInPlugin*,t_input,
					t_input_channel, t_input_value)));
}

void EWingInput::feedBack(t_input input, t_input_channel channel,
			  t_input_value value)
{
	EWing* ewing = device(input);
	if (ewing != NULL)
		ewing->feedBack(channel, value);
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(ewinginput, EWingInput)
