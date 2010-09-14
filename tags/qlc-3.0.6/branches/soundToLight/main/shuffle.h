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

#ifndef SHUFFLE_H
#define SHUFFLE_H

#include <QList>

#include <stdlib.h>
#include <time.h>

#include "function.h"
#include "state.h"
#include "stateFilter.h"

class QFile;
class QString;
class QDomDocument;

class Shuffle : public Function
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/** Constructor */
	Shuffle(QObject* parent);

	/** Destructor */
	virtual ~Shuffle();

private:
	Q_DISABLE_COPY(Shuffle)

	/*********************************************************************
	 * Function type
	 *********************************************************************/
public:
	Function::Type type() const;

	/*********************************************************************
	 * Copying
	 *********************************************************************/
public:
    /** @reimpl */
	Function* createCopy(Doc* doc);

	/** Copy the contents for this function from another function */
	bool copyFrom(const Function* function);

	/*********************************************************************
	 * Chaser contents
	 *********************************************************************/
public:
	/** Add the given function to the end of this chaser's step list */
	void addStep(t_function_id fid);

	/** Remove a function from the given step index */
	void removeStep(unsigned int index = 0);

	/** Get this chaser's list of steps */
	QList <State *> *steps() { return &m_steps; }

	/** Get this chaser's list of steps */
	QList <t_function_id> *sceneSteps();

protected:
	QList <State *> m_steps;
	QList <StateFilter *> m_filters;

	/*********************************************************************
	 * Edit
	 *********************************************************************/
public:
	/** Edit the function. Returns QDialog::DialogCode. */
	int edit(QWidget* parent);

	/*********************************************************************
	 * Save & Load
	 *********************************************************************/
public:
	/** Save this function to an XML document */
	bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

	/** Load this function contents from an XML document */
	bool loadXML(const QDomElement* root);

	/*********************************************************************
	 * Running
	 *********************************************************************/
public:
	void arm();
	void disarm();

	void start(MasterTimer* timer);
	void stop(MasterTimer* timer);

	bool write(QByteArray* universes);

protected:
	/** Increment or decrement the next function position */
	void nextStep();

	/** Start a step function at the given index */
	void startMemberAt(int index);

	/** Stop a step function at the given index */
	void stopMemberAt(int index);

protected:
	bool m_stopped;
	int m_runTimePosition;

	MasterTimer* m_masterTimer;
};

#endif
