	/*
  Q Light Controller
  shuffle.cpp

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
#include <QDebug>
#include <QFile>
#include <QtXml>

#include "common/qlcfixturedef.h"
#include "common/qlcfile.h"

#include "mastertimer.h"
#include "function.h"
#include "fixture.h"
#include "shuffle.h"
#include "doc.h"
#include "bus.h"

#include "blackoutState.h"
#include "sceneState.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Shuffle::Shuffle(QObject* parent) : Function(parent)
{
	m_masterTimer = NULL;

	setName(tr("New Shuffle"));
	setBus(Bus::defaultHold());

	/* initialize random seed: */
	srand ( time(NULL) );


	Doc* doc = qobject_cast <Doc*> (parent);
	if (doc != NULL)
	{
		/* Listen to function removals so that they can be removed from
		   this chaser as well. Parent might not always be Doc, but an
		   editor dialog, for example. Such chasers cannot be run,
		   though. */
		connect(doc, SIGNAL(functionRemoved(t_function_id)),
			this, SLOT(slotFunctionRemoved(t_function_id)));
	}

	//always add a blackout state to the start!
	BlackoutState *b = new BlackoutState();
	m_steps.append(b);
}

Shuffle::~Shuffle()
{
	m_steps.clear();
}

/*****************************************************************************
 * Function type
 *****************************************************************************/

Function::Type Shuffle::type() const
{
	return Function::Shuffle;
}

/*****************************************************************************
 * Copying
 *****************************************************************************/

Function* Shuffle::createCopy(Doc* doc)
{
	Q_ASSERT(doc != NULL);

	Function* copy = new Shuffle(doc);
	Q_ASSERT(copy != NULL);

	if (copy->copyFrom(this) == false)
	{
		delete copy;
		copy = NULL;
	}
	else if (doc->addFunction(copy) == false)
	{
		delete copy;
		copy = NULL;
	}
	else
	{
		copy->setName(tr("Copy of %1").arg(name()));
	}
	
	return copy;
}

bool Shuffle::copyFrom(const Function* function)
{
	const Shuffle* shuffle = qobject_cast<const Shuffle*> (function);
	if (shuffle == NULL)
		return false;

	m_steps.clear();
	m_steps = shuffle->m_steps;

	bool result = Function::copyFrom(function);

	emit changed(m_id);

	return result;
}

/*****************************************************************************
 * Contents
 *****************************************************************************/

void Shuffle::addStep(t_function_id id)
{
	m_steps.append(new SceneState(id));

	emit changed(m_id);
}

void Shuffle::removeStep(unsigned int index)
{
	Q_ASSERT(int(index) < m_steps.size());
	m_steps.removeAt(index);

	emit changed(m_id);
}

QList <t_function_id> *Shuffle::sceneSteps()
{
	QList <t_function_id> *list;

	list = new QList <t_function_id>();
	QListIterator <State *> it(m_steps);
	while (it.hasNext() == true)
	{
		State *state;
		state = it.next();

		if (SceneState* scenestate = dynamic_cast<SceneState*>(state)) {
			list->append(scenestate->functionId());
		}
	}

	return list;
}

/*****************************************************************************
 * Save & Load
 *****************************************************************************/

bool Shuffle::saveXML(QDomDocument* doc, QDomElement* wksp_root)
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
	root.setAttribute(KXMLQLCFunctionType, Function::typeToString(type()));
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
	QListIterator <t_function_id> it(*sceneSteps());
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

bool Shuffle::loadXML(const QDomElement* root)
{
	t_fixture_id step_fxi = KNoID;
	int step_number = 0;

	QDomNode node;
	QDomElement tag;

	m_steps.clear();

	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCFunction)
	{
		qDebug() << "Function node not found!";
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
			setBus(tag.text().toInt());
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

			if (step_number >= m_steps.size())
				addStep(step_fxi);
			else
				m_steps.insert(step_number,
					       new SceneState(step_fxi));
		}
		else
		{
			qDebug() << "Unknown chaser tag:" << tag.tagName();
		}

		node = node.nextSibling();
	}


	//always add a blackout state to the end!
	BlackoutState *b = new BlackoutState();
	m_steps.append(b);

	return true;
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void Shuffle::arm()
{
}

void Shuffle::disarm()
{
}

void Shuffle::start(MasterTimer* timer)
{
	Q_ASSERT(timer != NULL);

	m_masterTimer = timer;

	/* No point running this shuffle if it has no steps */
	if (m_steps.count() == 0)
	{
		emit stopped(m_id);
		return;
	}

	/* Current position starts from invalid index because nextStep() is
	   called first in write(). */
	m_runTimePosition = -1;
	Function::start(timer);
}

void Shuffle::stop(MasterTimer* timer)
{
	Q_ASSERT(timer != NULL);

	stopMemberAt(m_runTimePosition);
	Function::stop(timer);
	m_masterTimer = NULL;
}

bool Shuffle::write(QByteArray* universes)
{
	Q_UNUSED(universes);

	/* TODO: With some changes to scene, shuffle could basically act as a
	   proxy for its members by calling the scene's write() functions
	   by itself instead of starting/stopping them. Whether this would do
	   any good, is not clear. */

	if (m_elapsed == 0)
	{
		/* Get the next step index */
		nextStep();

		/* Start the current function */
		startMemberAt(m_runTimePosition);

		/* Mark one cycle being elapsed */
		m_elapsed = 1;
	}
	else if (m_elapsed >= Bus::instance()->value(m_busID))
	{
		/* Stop current function */
		stopMemberAt(m_runTimePosition);

		/* Get the next step index */
		nextStep();

		/* Start the next step immediately */
		startMemberAt(m_runTimePosition);

		/* Mark one cycle being elapsed */
		m_elapsed = 1;
	}
	else
	{
		/* Just waiting for hold time to pass */
		m_elapsed++;
	}

	return true;
}


void Shuffle::nextStep()
{
	int totalWeight = 0;

	QListIterator <State *> it(m_steps);
	while (it.hasNext() == true) {
		State *state = it.next();
		totalWeight += state->baseWeight();
	}

	int randNo = rand() % totalWeight;

	for (int i = 0; i < m_steps.size(); i++) {
		State *state = m_steps[i];
		randNo -= state->baseWeight();
		if (randNo < 0) {
			m_runTimePosition = i;
			break;
		}
	}
}

void Shuffle::startMemberAt(int index)
{
	State* state;

	state = m_steps.at(index);
	Q_ASSERT(index != KNoID);

	state->start();
}

void Shuffle::stopMemberAt(int index)
{
	State* state;

	state = m_steps.at(index);
	Q_ASSERT(index != KNoID);

	//TODO
	state->stop();
}
