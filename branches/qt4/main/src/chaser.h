/*
  Q Light Controller
  chaser.h
  
  Copyright (C) Heikki Junnila
  
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

#ifndef CHASER_H
#define CHASER_H

#include <QList>
#include "function.h"

class Event;
class QFile;
class QString;
class QDomDocument;

class Chaser : public Function
{
	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/** Constructor */
	Chaser();

	/**
	 * Copy the contents of the given chaser into this chaser.
	 *
	 * @param ch Chaser to copy from
	 * @param append If true, existing contents will not be cleared;
	 *               new steps appear after existing steps. If false,
	 *               the existing steps of this chaser are cleared.
	 */
	void copyFrom(Chaser* ch, bool append = false);

	/** Destructor */
	virtual ~Chaser();

	/** Chasers are not attached to a particular fixture */
	bool setFixtureInstance(t_fixture_id) { return false; }

	/*********************************************************************
	 * Chaser contents
	 *********************************************************************/
public:
	/** Add the given function to the end of this chaser's step list */
	void addStep(t_function_id);

	/** Remove a function from the given step index */
	void removeStep(unsigned int index = 0);

	/** Raise the given step once (move it one step earlier) */
	bool raiseStep(unsigned int index);

	/** Lower the given step once (move it one step later) */
	bool lowerStep(unsigned int index);

	/** Set this chaser's running order */
	void setRunOrder(RunOrder ro);

	/** Get this chaser's running order */
	RunOrder runOrder() { return m_runOrder; }

	/** Set this chaser's initial running direction */
	void setDirection(Direction dir);

	/** Get this chaser's initial running direction */
	Direction direction() { return m_direction; }

	/** Get this chaser's list of steps */
	QList <t_function_id> *steps() { return &m_steps; }

protected:
	QList <t_function_id> m_steps;

	RunOrder m_runOrder;
	Direction m_direction;

	/*********************************************************************
	 * Save & Load
	 *********************************************************************/
public:
	/** Save this function to an XML document */
	bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

	/** Load this function contents from an XML document */
	bool loadXML(QDomDocument* doc, QDomElement* root);

	/*********************************************************************
	 * Running
	 *********************************************************************/
public:
	/** Initiate a speed change (from a speed bus) */
	void busValueChanged(t_bus_id, t_bus_value);

	/** Allocate everything needed in run-time */
	void arm();

	/** Delete everything needed in run-time */
	void disarm();

	/** Stop this function */
	void stop();

	/** Do some post-run cleanup (called by FunctionConsumer) */
	void cleanup();

	/**
	 * Currently running child function calls this function when it is
	 * ready. This wakes up the chaser producer thread.
	 */
	void childFinished();

protected:
	/** Initialize some run-time values */
	void init();

	/** Main producer thread */
	void run();

	/** Start a step function at the given index */
	bool startMemberAt(int index);

	/** Stop a step function at the given index */
	void stopMemberAt(int index);

	/** Wait until m_holdTime ticks (1/Hz) have elapsed */
	void hold();

protected:
	bool m_childRunning;

	t_bus_value m_holdTime;
	t_bus_value m_holdStart;
	t_bus_value m_timeCode;

	Direction m_runTimeDirection;
	int m_runTimePosition;
};

#endif
