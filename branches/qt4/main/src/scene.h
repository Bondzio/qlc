/*
  Q Light Controller
  scene.h
  
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

#ifndef SCENE_H
#define SCENE_H

#include <QList>

#include "function.h"
#include "common/qlctypes.h"

class EventBuffer;
class RunTimeData;
class Fixture;
class Scene;

class QDomDocument;
class QDomElement;

#define KXMLQLCSceneValue "Value"
#define KXMLQLCSceneValueFixture "Fixture"
#define KXMLQLCSceneValueChannel "Channel"

/*****************************************************************************
 * SceneValue
 *****************************************************************************/

class SceneValue;
class SceneValue
{
public:
	/** Normal constructor */
	SceneValue(t_fixture_id fxi_id, t_channel channel, t_value value);

	/** Copy constructor */
	SceneValue(const SceneValue& scv);

	/** Load constructor */
	SceneValue(QDomElement* tag);

	/** Destructor */
	~SceneValue();

	/** A SceneValue is not valid if .fxi == KNoID */
	bool isValid();

	/** Comparator function for qSort() */
	bool operator< (const SceneValue& scv) const;

	/** Comparator function for matching SceneValues */
	bool operator== (const SceneValue& scv) const;

	/** Save this SceneValue to XML file */
	bool saveXML(QDomDocument* doc, QDomElement* scene_root) const;

public:
	t_fixture_id fxi;
	t_channel channel;
	t_value value;
};

/*****************************************************************************
 * Scene
 *****************************************************************************/

class Scene : public Function
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	Scene(QObject* parent);
	~Scene();

	void copyFrom(Scene* ch);
	
	/*********************************************************************
	 * Values
	 *********************************************************************/
public:
	void setValue(SceneValue scv);
	void setValue(t_fixture_id fxi, t_channel ch, t_value value);
	void unsetValue(t_fixture_id fxi, t_channel ch);
	SceneValue value(t_fixture_id fxi, t_channel ch);

	QList <SceneValue> *values() { return &m_values; }

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:	
	bool saveXML(QDomDocument* doc, QDomElement* wksp_root);
	bool loadXML(QDomDocument* doc, QDomElement* root);
	
	/*********************************************************************
	 * Bus
	 *********************************************************************/
public slots:
	void slotBusValueChanged(t_bus_id id, t_bus_value value);

public:
	void speedChange(t_bus_value value);
	
	/*********************************************************************
	 * Running
	 *********************************************************************/
public:
	void arm();
	void disarm();
	void stop();
	
protected:
	void run();
	
protected:
	QList <SceneValue> m_values;
	
	t_bus_value m_timeSpan;
	t_bus_value m_elapsedTime;
	
	RunTimeData* m_runTimeData;
	t_buffer_data* m_channelData;

	bool m_stopped;
};

class RunTimeData
{
public:
	t_channel address;

	float start;
	float current;
	float target;

	bool ready;
};

#endif
