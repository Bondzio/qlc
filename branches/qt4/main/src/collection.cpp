/*
  Q Light Controller
  collection.cpp
  
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
#include <QString>
#include <QFile>
#include <QList>
#include <QtXml>

#include "common/qlcfile.h"

#include "functionconsumer.h"
#include "eventbuffer.h"
#include "collection.h"
#include "function.h"
#include "app.h"
#include "doc.h"

extern App* _app;

using namespace std;

Collection::Collection() : Function(Function::Collection)
{
	m_childCount = 0;
}

void Collection::copyFrom(Collection* fc, bool append)
{
	Q_ASSERT(fc != NULL);

	Function::setName(fc->name());
	Function::setBus(fc->busID());

	if (append == false)
		m_steps.clear();

	QListIterator <t_function_id> it(fc->m_steps);
	while (it.hasNext() == true)
		m_steps.append(it.next());
}

Collection::~Collection()
{
	stop();

	while (m_running == true)
		pthread_yield();

	m_steps.clear();
}

bool Collection::saveXML(QDomDocument* doc, QDomElement* wksp_root)
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

bool Collection::loadXML(QDomDocument* doc, QDomElement* root)
{
	t_fixture_id step_fxi = KNoID;
	
	QDomNode node;
	QDomElement tag;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCFunction)
	{
		cout << "Function node not found!" << endl;
		return false;
	}

	/* Load collection contents */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		
		if (tag.tagName() == KXMLQLCFunctionStep)
		{
			step_fxi = tag.text().toInt();
			m_steps.append(step_fxi);
		}
		else
		{
			cout << "Unknown collection tag: "
			     << tag.tagName().toStdString()
			     << endl;
		}

		node = node.nextSibling();
	}

	return true;
}

bool Collection::addItem(t_function_id id)
{
	m_startMutex.lock();

	if (m_running == false)
	{
		m_steps.append(id);
		m_startMutex.unlock();
		return true;
	}

	m_startMutex.unlock();
	return false;
}

bool Collection::removeItem(t_function_id id)
{
	m_startMutex.lock();

	if (m_running == false)
	{
		m_steps.takeAt(m_steps.indexOf(id));
		m_startMutex.unlock();
		return true;
	}

	m_startMutex.unlock();
	return false;
}

void Collection::speedChange()
{
}

void Collection::arm()
{
	if (m_eventBuffer == NULL)
		m_eventBuffer = new EventBuffer(0, 0);
}

void Collection::disarm()
{
	if (m_eventBuffer != NULL)
		delete m_eventBuffer;
	m_eventBuffer = NULL;
}

void Collection::stop()
{
	/* TODO: this stops these functions, regardless of whether they
	   were started by this collection or not */
	QListIterator <t_function_id> it(m_steps);
	while (it.hasNext() == true)
	{
		Function* function = _app->doc()->function(it.next());
		if (function != NULL)
			function->stop();
	}
}

void Collection::cleanup()
{
	Q_ASSERT(m_childCount == 0);

	m_stopped = false;

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

	m_startMutex.lock();
	m_running = false;
	m_startMutex.unlock();
}


void Collection::childFinished()
{
	m_childCountMutex.lock();
	m_childCount--;
	m_childCountMutex.unlock();
}

void Collection::init()
{
	m_childCountMutex.lock();
	m_childCount = 0;
	m_childCountMutex.unlock();

	m_stopped = false;
	m_removeAfterEmpty = false;

	// Append this function to running functions list
	_app->functionConsumer()->cue(this);
}

void Collection::run()
{
	QListIterator <t_function_id> it(m_steps);

	// Calculate starting values and set this function to functionconsumer
	init();
  
	while (it.hasNext() == true)
	{
		Function* function = _app->doc()->function(it.next());
		if (function != NULL && function->engage(this))
		{
			m_childCountMutex.lock();
			m_childCount++;
			m_childCountMutex.unlock();
		}
	}

	// Wait for all children to stop
	while (m_childCount > 0)
		pthread_yield();

	m_removeAfterEmpty = true;
}
