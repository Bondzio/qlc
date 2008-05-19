/*
  Q Light Controller
  scene.cpp

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
#include <iostream>
#include <QList>
#include <QFile>
#include <QtXml>

#include <time.h>

#include "common/qlcfixturedef.h"
#include "common/qlcfile.h"

#include "functionconsumer.h"
#include "eventbuffer.h"
#include "dmxmap.h"
#include "scene.h"
#include "app.h"
#include "doc.h"
#include "bus.h"

using namespace std;

extern App* _app;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Scene::Scene(QObject* parent) : Function(parent, Function::Scene)
{
	m_values = NULL;
	m_timeSpan = 255;
	m_elapsedTime = 0;
	m_runTimeData = NULL;
	m_channelData = NULL;
	
	setBus(KBusIDDefaultFade);
}

Scene::~Scene()
{
	if (m_values != NULL)
		delete [] m_values;
}

void Scene::copyFrom(Scene* sc, t_fixture_id to)
{
	Q_ASSERT(sc != NULL);
	
	Function::setName(sc->name());
	Function::setBus(sc->busID());
	
	setFixture(to);
	
	if (m_values != NULL)
		delete [] m_values;
	m_values = new SceneValue[m_channels];
	
	for (t_channel ch = 0; ch < m_channels; ch++)
	{
		m_values[ch].value = sc->m_values[ch].value;
		m_values[ch].type = sc->m_values[ch].type;
	}
}

/*****************************************************************************
 * Fixture
 *****************************************************************************/

void Scene::setFixture(t_fixture_id id)
{
	Fixture* fxi = NULL;

	fxi = _app->doc()->fixture(id);
	if (fxi == NULL)
		return;
	
	t_channel newChannels = fxi->channels();
	
	if (m_channels == 0)
	{
		m_channels = newChannels;
		
		m_values = new SceneValue[m_channels];
		
		for (t_channel i = 0; i < m_channels; i++)
		{
			m_values[i].value = 0;
			m_values[i].type = Fade;
		}
	}
	else
	{
		Q_ASSERT(m_channels == newChannels);
	}
	
	m_fixture = id;
	
	_app->doc()->setModified();
	_app->doc()->emitFunctionChanged(m_id);
}

/*****************************************************************************
 * Values
 *****************************************************************************/

bool Scene::set(t_channel ch, t_value value, ValueType type)
{
	if (ch < m_channels)
	{
		m_values[ch].value = value;
		m_values[ch].type = type;
		
		_app->doc()->setModified();
		_app->doc()->emitFunctionChanged(m_id);

		return true;
	}
	else
	{
		return false;
	}
}

SceneValue Scene::channelValue(t_channel ch)
{
	return m_values[ch];
}

Scene::ValueType Scene::valueType(t_channel ch)
{
	Q_ASSERT(ch < m_channels);
	return m_values[ch].type;
}

QString Scene::valueTypeString(t_channel ch)
{
	switch(m_values[ch].type)
	{
	case Set:
		return QString("Set");
		break;
		
	case Fade:
		return QString("Fade");
		break;
		
	default:
	case NoSet:
		return QString("NoSet");
		break;
	}
}

Scene::ValueType Scene::stringToValueType(QString type)
{
	if (type == QString("Set"))
		return Scene::Set;
	else if (type == QString("Fade"))
		return Scene::Fade;
	else
		return Scene::NoSet;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool Scene::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
	Fixture* fxi = NULL;
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(wksp_root != NULL);

	fxi = _app->doc()->fixture(m_fixture);
	Q_ASSERT(fxi != NULL);

	/* Function tag */
	root = doc->createElement(KXMLQLCFunction);
	wksp_root->appendChild(root);

	root.setAttribute(KXMLQLCFunctionID, id());
	root.setAttribute(KXMLQLCFunctionType, Function::typeToString(m_type));
	root.setAttribute(KXMLQLCFunctionName, name());
	root.setAttribute(KXMLQLCFunctionFixture, fixture());

	/* Speed bus */
	tag = doc->createElement(KXMLQLCBus);
	root.appendChild(tag);
	tag.setAttribute(KXMLQLCBusRole, KXMLQLCBusFade);
	str.setNum(busID());
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Scene contents */
	for (t_channel i = 0; i < fxi->channels(); i++)
	{
		/* Value tag */
		tag = doc->createElement(KXMLQLCFunctionValue);
		
		/* Value type & channel */
		tag.setAttribute(KXMLQLCFunctionChannel, i);
		tag.setAttribute(KXMLQLCFunctionValueType, valueTypeString(i));
		root.appendChild(tag);

		/* Value contents */
		str.setNum(m_values[i].value);
		text = doc->createTextNode(str);
		tag.appendChild(text);
	}
}

bool Scene::loadXML(QDomDocument* doc, QDomElement* root)
{
	t_value value = 0;
	t_channel ch = 0;
	Scene::ValueType value_type;
	QString str;
	
	QDomNode node;
	QDomElement tag;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCFunction)
	{
		qWarning("Function node not found!");
		return false;
	}

	/* Load scene contents */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		
		if (tag.tagName() == KXMLQLCBus)
		{
			/* Bus */
			str = tag.attribute(KXMLQLCBusRole);
			Q_ASSERT(str == KXMLQLCBusFade);

			setBus(tag.text().toInt());
		}
		else if (tag.tagName() == KXMLQLCFunctionValue)
		{
			/* Channel value */
			str = tag.attribute(KXMLQLCFunctionValueType);
			value_type = stringToValueType(str);
			ch = tag.attribute(KXMLQLCFunctionChannel).toInt();
			value = tag.text().toInt();

			Q_ASSERT(set(ch, value, value_type) == true);
		}
		else
			cout << "Unknown scene tag: "
			     << tag.tagName().toStdString()
			     << endl;
		
		node = node.nextSibling();
	}

	return true;
}

/*****************************************************************************
 * Bus
 *****************************************************************************/

void Scene::slotBusValueChanged(t_bus_id id, t_bus_value value)
{
	if (id != m_busID)
		return;

	if (isRunning() == true)
		speedChange(value);
	else
		m_timeSpan = value;
}

void Scene::speedChange(t_bus_value newTimeSpan)
{
	m_timeSpan = newTimeSpan;
	if (m_timeSpan == 0)
	{
		m_timeSpan = static_cast<t_bus_value>
			(1.0 / static_cast<float> (KFrequency));
	}
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void Scene::arm()
{
	if (m_runTimeData == NULL)
		m_runTimeData = new RunTimeData[m_channels];

	if (m_channelData == NULL)
		m_channelData = new t_buffer_data[m_channels];
	
	if (m_eventBuffer == NULL)
		m_eventBuffer = new EventBuffer(m_channels, KFrequency >> 1);
}

void Scene::disarm()
{
	if (m_runTimeData != NULL)
		delete [] m_runTimeData;
	m_runTimeData = NULL;
	
	if (m_channelData != NULL)
		delete [] m_channelData;
	m_channelData = NULL;
	
	if (m_eventBuffer != NULL)
		delete m_eventBuffer;
	m_eventBuffer = NULL;
}

void Scene::stop()
{
	m_stopped = true;
}

void Scene::run()
{
	t_channel ch = 0;
	t_channel ready = 0;
	t_channel address = 0;
	
	// Initialize this scene for running
	m_stopped = false;
	
	// Fetch the fixture address for run time access.
	address = _app->doc()->fixture(fixture())->universeAddress();

	// No time has yet passed for this scene.
	m_elapsedTime = 0;
	
	// Get speed
	m_timeSpan = Bus::value(m_busID);
	
	// Set initial speed
	speedChange(m_timeSpan);

	// Append this function to running functions' list
	_app->functionConsumer()->cue(this);
	
	// Check if this scene needs to play and fill some stuff just in case
	for (t_channel i = 0; i < m_channels; i++)
	{
		if (m_values[i].type == Set || m_values[i].type == Fade)
		{
			if (m_values[i].value == (int) m_runTimeData[i].current)
				ready++;
		}
		else
		{
			// NoSet values are treated as ready
			ready++;
		}

		/* Fill run-time buffers with the data for the first step */
		m_runTimeData[i].current = m_runTimeData[i].start =
			static_cast<float> 
			(_app->dmxMap()->getValue(address + i));
		
		m_runTimeData[i].target = 
			static_cast<float> (m_values[i].value);
		
		m_runTimeData[i].ready = false;

	}
	
	// This scene does not need to be played because all target
	// values are already where they are supposed to be.
	if (ready == m_channels)
		m_stopped = true;

	for (m_elapsedTime = 0;
	     m_elapsedTime < m_timeSpan && m_stopped == false;
	     m_elapsedTime++)
	{
		for (ch = 0; ch < m_channels; ch++)
		{
			if (m_values[ch].type == NoSet || 
			    m_runTimeData[ch].ready)
			{
				m_channelData[ch] = KChannelInvalid << 8;
				m_channelData[ch] |= 0;
				
				// This channel contains a value that is not
				// supposed to be written (anymore, in case of
				// a ready value, which comes from "set" type)
				continue;
			}
			else if (m_values[ch].type == Set)
			{
				// Just set the target value
				m_channelData[ch] = (address + ch) << 8;
				m_channelData[ch] |= static_cast<t_buffer_data>
					(m_values[ch].value);
				
				// ...and don't touch this channel anymore
				m_runTimeData[ch].ready = true;
			}
			else if (m_values[ch].type == Fade)
			{
				// Calculate the current value based on what
				// it should be after m_elapsedTime to be
				// ready at m_timeSpan
				m_runTimeData[ch].current = 
					m_runTimeData[ch].start
					+ (m_runTimeData[ch].target 
					   - m_runTimeData[ch].start)
					* ((float)m_elapsedTime / m_timeSpan);
				
				m_channelData[ch] = (address + ch) << 8;
				
				m_channelData[ch] |= static_cast<t_buffer_data>
					(m_runTimeData[ch].current);
			}
		}
		
		m_eventBuffer->put(m_channelData);
	}
	
	// Write the last step exactly to target because timespan might have
	// been set to a smaller amount than what has elapsed. Also, because
	// floats are NEVER exact numbers, it might be that we never quite reach
	// the target within the given timespan (in case values don't add up).
	for (ch = 0; ch < m_channels; ch++)
	{
		if (m_values[ch].type == NoSet || m_runTimeData[ch].ready)
		{
			m_channelData[ch] = KChannelInvalid << 8;
			m_channelData[ch] |= 0;
		}
		else
		{
			// Just set the target value
			m_channelData[ch] = (address + ch) << 8;
			m_channelData[ch] |= 
				static_cast<t_buffer_data> (m_values[ch].value);
			
			// ...and don't touch this channel anymore
			m_runTimeData[ch].ready = true;
		}
	}
	
	if (m_stopped == false)
	{
		/* Put the last values to the buffer */
		m_eventBuffer->put(m_channelData);
	}
	else
	{
		//
		// This scene was stopped. Clear buffer so that this function
		// can finish as quickly as possible
		//
		m_eventBuffer->purge();
	}

	emit stopped(m_id);
}
