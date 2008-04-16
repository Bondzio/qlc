/*
  Q Light Controller
  chaser.cpp
  
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
#include <QFile>
#include <QtXml>

#include "common/qlcfixturedef.h"
#include "common/qlcfile.h"

#include "functionconsumer.h"
#include "eventbuffer.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"
#include "app.h"
#include "bus.h"

extern App* _app;

using namespace std;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Chaser::Chaser() : Function(Function::Chaser)
{
	m_runOrder = Loop;
	m_direction = Forward;
	m_childRunning = false;

	m_holdTime = 0;
	m_holdStart = 0;
	m_timeCode = 0;

	m_runTimeDirection = Forward;
	m_runTimePosition = 0;

	setBus(KBusIDDefaultHold);
}

void Chaser::copyFrom(Chaser* ch, bool append)
{
	Q_ASSERT(ch != NULL);

	Function::setName(ch->name());
	setDirection(ch->direction());
	setRunOrder(ch->runOrder());

	if (append == false)
		m_steps.clear();

	QListIterator <t_function_id> it(ch->m_steps);
	while (it.hasNext() == true)
		m_steps.append(it.next());
}

Chaser::~Chaser()
{
	stop();
	m_steps.clear();
}

/*****************************************************************************
 * Contents
 *****************************************************************************/

void Chaser::addStep(t_function_id id)
{
	m_startMutex.lock();

	if (m_running == false)
	{
		m_steps.append(id);
		_app->doc()->setModified();
	}
	else
	{
		cout << "Chaser is running. Cannot modify steps!" << endl;
	}  

	m_startMutex.unlock();
}

void Chaser::removeStep(int index)
{
	Q_ASSERT(((unsigned int)index) < m_steps.count());

	m_startMutex.lock();

	if (m_running == false)
	{
		m_steps.removeAt(index);
		_app->doc()->setModified();
	}
	else
	{
		cout << "Chaser is running. Cannot modify steps!" << endl;
	}

	m_startMutex.unlock();
}

bool Chaser::raiseStep(unsigned int index)
{
	bool result = false;

	m_startMutex.lock();

	if (m_running == false)
	{
		if (index > 0 && index < m_steps.count())
		{
			t_function_id fid = m_steps.takeAt(index);
			m_steps.insert(index - 1, fid);

			_app->doc()->setModified();
			result = true;
		}
	}
	else
	{
		cout << "Chaser is running. Cannot modify steps!" << endl;
	}

	m_startMutex.unlock();

	return result;
}

bool Chaser::lowerStep(unsigned int index)
{
	bool result = false;

	m_startMutex.lock();

	if (m_running == false)
	{
		if (index < m_steps.count() - 1 && index >= 0)
		{
			t_function_id fid = m_steps.takeAt(index);
			m_steps.insert(index + 1, fid);

			_app->doc()->setModified();
			result = true;
		}
	}
	else
	{
		cout << "Chaser is running. Cannot modify steps!" << endl;
	}

	m_startMutex.unlock();

	return result;
}

void Chaser::setRunOrder(RunOrder ro)
{
	m_runOrder = ro;
}

void Chaser::setDirection(Direction dir)
{
	m_direction = dir;
}

/*****************************************************************************
 * Save & Load
 *****************************************************************************/

bool Chaser::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;
	int i = 0;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(wksp_root != NULL);

	/* Function tag */
	root = doc->createElement(KXMLQLCFunction);
	wksp_root->appendChild(root);

	root.setAttribute(KXMLQLCFunctionID, id());
	root.setAttribute(KXMLQLCFunctionType, Function::typeToString(m_type));
	root.setAttribute(KXMLQLCFunctionFixture, fixture());
	root.setAttribute(KXMLQLCFunctionName, name());

	/* Speed bus */
	tag = doc->createElement(KXMLQLCBus);
	root.appendChild(tag);
	tag.setAttribute(KXMLQLCBusRole, KXMLQLCBusHold);
	str.setNum(busID());
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Direction */
	tag = doc->createElement(KXMLQLCFunctionDirection);
	root.appendChild(tag);
	text = doc->createTextNode(Function::directionToString(m_direction));
	tag.appendChild(text);

	/* Run order */
	tag = doc->createElement(KXMLQLCFunctionRunOrder);
	root.appendChild(tag);
	text = doc->createTextNode(Function::runOrderToString(m_runOrder));
	tag.appendChild(text);

	/* Steps */
	QListIterator <t_function_id> it(m_steps);
	while (it.hasNext() == true)
	{
		/* Step tag */
		tag = doc->createElement(KXMLQLCFunctionStep);
		root.appendChild(tag);

		/* Step number */
		tag.setAttribute(KXMLQLCFunctionNumber, i++);

		/* Step Function ID */
		str.setNum(it.next());
		text = doc->createTextNode(str);
		tag.appendChild(text);
	}

	return true;
}

bool Chaser::loadXML(QDomDocument* doc, QDomElement* root)
{
	t_fixture_id step_fxi = KNoID;
	int step_number = 0;
	QString str;
	
	QDomNode node;
	QDomElement tag;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCFunction)
	{
		cout << "Function node not found!" << endl;
		return false;
	}

	/* Load chaser contents */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		
		if (tag.tagName() == KXMLQLCBus)
		{
			/* Bus */
			str = tag.attribute(KXMLQLCBusRole);
			Q_ASSERT(str == KXMLQLCBusHold);

			Q_ASSERT(setBus(tag.text().toInt()) == true);
		}
		else if (tag.tagName() == KXMLQLCFunctionDirection)
		{
			/* Direction */
			setDirection(Function::stringToDirection(tag.text()));
		}
		else if (tag.tagName() == KXMLQLCFunctionRunOrder)
		{
			/* Run Order */
			setRunOrder(Function::stringToRunOrder(tag.text()));
		}
		else if (tag.tagName() == KXMLQLCFunctionStep)
		{
			step_number = 
				tag.attribute(KXMLQLCFunctionNumber).toInt();
			step_fxi = tag.text().toInt();

			if (step_number > m_steps.size())
				m_steps.append(step_fxi);
			else
				m_steps.insert(m_steps.at(step_number),
					       step_fxi);
			
		}
		else
		{
			cout << "Unknown chaser tag: "
			     << tag.tagName().toStdString()
			     << endl;
		}
		
		node = node.nextSibling();
	}

	return true;
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void Chaser::busValueChanged(t_bus_id id, t_bus_value value)
{
	if (id == m_busID)
	{
		m_holdTime = value;
	}
}

void Chaser::arm()
{
	// There's actually no need for an eventbuffer, but
	// because FunctionConsumer does EventBuffer::get() calls, it must be
	// there... So allocate a zero length buffer.
	if (m_eventBuffer == NULL)
		m_eventBuffer = new EventBuffer(0, 0);
}

void Chaser::disarm()
{
	if (m_eventBuffer) delete m_eventBuffer;
	m_eventBuffer = NULL;
}

void Chaser::stop()
{
	Function::stop();
}

void Chaser::cleanup()
{
	if (m_virtualController)
	{
		QApplication::postEvent(m_virtualController,
					new FunctionStopEvent(m_id));

		m_virtualController = NULL;
	}

	if (m_parentFunction)
	{
		m_parentFunction->childFinished();
		m_parentFunction = NULL;
	}

	m_stopped = false;

	m_startMutex.lock();
	m_running = false;
	m_startMutex.unlock();
}

void Chaser::childFinished()
{
	m_childRunning = false;
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void Chaser::init()
{
	m_childRunning = false;
	m_removeAfterEmpty = false;
	m_stopped = false;

	// Get speed
	Bus::value(m_busID, m_holdTime);

	// Add this to function consumer
	_app->functionConsumer()->cue(this);
}

void Chaser::run()
{
	// Calculate starting values
	init();

	m_runTimeDirection = m_direction;

	if (m_runTimeDirection == Forward)
		m_runTimePosition = 0;
	else
		m_runTimePosition = m_steps.count() - 1;

	while (m_stopped == false)
	{
		// Run thru this chaser, either forwards or backwards
		if (m_runTimeDirection == Forward)
		{
			while (m_runTimePosition < (int) m_steps.count() &&
			       m_stopped == false)
			{
				m_childRunning =
					startMemberAt(m_runTimePosition);
	      
				// Wait for child to complete or stop signal
				while (m_childRunning == true &&
				       m_stopped == false)
				{
					// sched_yield();
				}

				if (m_stopped == true)
				{
					stopMemberAt(m_runTimePosition);
					break;
				}
				else
				{
					// Wait for m_holdTime
					hold();
					m_runTimePosition++;
				}
			}
		}
		else
		{
			while (m_runTimePosition >= 0 && !m_stopped)
			{
				m_childRunning =
					startMemberAt(m_runTimePosition);

				// Wait for child to complete or stop signal
				while (m_childRunning == true &&
				       m_stopped == false)
				{
					// sched_yield;
				}

				if (m_stopped == true)
				{
					stopMemberAt(m_runTimePosition);
					break;
				}
				else
				{
					// Wait for m_holdTime
					hold();
					m_runTimePosition--;
				}
			}
		}

		// Check what should be done after one round
		if (m_runOrder == SingleShot)
		{
			// That's it
			break;
		}
		else if (m_runOrder == Loop)
		{
			// Just continue as before, start from the beginning
			m_runTimePosition = 0;
			continue;
		}
		else // if (m_runOrder == PingPong)
		{
			// Change run order
			if (m_runTimeDirection == Forward)
			{
				m_runTimeDirection = Backward;
	      
				// -2: Don't run the last function again
				m_runTimePosition = m_steps.count() - 2; 
			}
			else
			{
				m_runTimeDirection = Forward;

				// 1: Don't run the first function again
				m_runTimePosition = 1; 
			}
		}
	}

	// This chaser can be removed from the list after the buffer is empty.
	// (meaning immediately because this doesn't produce any events).
	m_removeAfterEmpty = true;
}

bool Chaser::startMemberAt(int index)
{
	t_function_id id = m_steps.at(index);
  	Function* function = _app->doc()->function(id);
	if (function == NULL)
		return false;
  
	if (function->engage(this))
		return true;
	else
		return false;
}

void Chaser::stopMemberAt(int index)
{
	t_function_id id = m_steps.at(index);
	Function* function = _app->doc()->function(id);
	if (function != NULL)
		function->stop();
}

void Chaser::hold()
{
	/* Don't engage sleeping at all if holdtime is zero */
	if (m_holdTime > 0)
	{
		m_holdStart = _app->functionConsumer()->timeCode();
		while (m_stopped == false)
		{
			m_timeCode = _app->functionConsumer()->timeCode();
			if ((m_timeCode - m_holdStart) >= m_holdTime)
				break;
		}
	}
}
