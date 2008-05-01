/*
  Q Light Controller
  collection.h
  
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

#ifndef COLLECTION_H
#define COLLECTION_H

#include <QMutex>
#include <QList>

#include "function.h"

class QDomDocument;

class Collection : public Function
{
	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	Collection();
	virtual ~Collection();

	/** Copy given function's contents to this */
	void copyFrom(Collection* fc, bool append = false);

	/** Collections don't belong to any particular fixture */
	bool setFixtureInstance(t_fixture_id) { return false; }

	/*********************************************************************
	 * Contents
	 *********************************************************************/
public:
	/** Add a function to this collection */
	bool addItem(t_function_id);

	/** Remove a function from this collection */
	bool removeItem(t_function_id);

	/** Get this function's list of steps */
	QList <t_function_id> *steps() { return &m_steps; }

protected:
	QList <t_function_id> m_steps;

	/*********************************************************************
	 * Running
	 *********************************************************************/
public:
	/** Initiate a speed change (from a speed bus) */
	void speedChange();

	/** Allocate some stuff for run-time */
	void arm();

	/** Delete run-time allocations */
	void disarm();

	/** Do some post-run cleanup */
	void cleanup();

	/** Stop this function */
	void stop();
	
	/** Called whenever one of this function's child functions stops */
	void childFinished();

protected:
	/** Initialize this collection to run */
	void init();

	/** Main producer thread */
	void run();

protected:
	int m_childCount;
	QMutex m_childCountMutex;

	/*********************************************************************
	 * Save & Load
	 *********************************************************************/
public:
	/** Save function's contents to an XML document */
	bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

	/** Load function's contents from an XML document */
	bool loadXML(QDomDocument* doc, QDomElement* root);
};

#endif
