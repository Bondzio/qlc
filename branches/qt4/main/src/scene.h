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

#include "function.h"
#include "common/qlctypes.h"

class EventBuffer;
class RunTimeData;
class SceneValue;
class Fixture;
class Scene;

class QDomDocument;
class QDomElement;

class Scene : public Function
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	Scene(QObject* parent);
	~Scene();
	
	/** Copy scene contents and assign it to a fixture */
	void copyFrom(Scene* sc, t_fixture_id to);

	/*********************************************************************
	 * Fixture
	 *********************************************************************/
public:
	/** Set the fixture that this scene is assigned to */
	void setFixture(t_fixture_id id);

	/*********************************************************************
	 * Values
	 *********************************************************************/
public:
	enum ValueType
	{
		Set   = 0, // Normal value
		Fade  = 1, // Fade value
		NoSet = 2  // Ignored value
	};

	SceneValue* values() { return m_values; }
	
	bool set(t_channel ch, t_value value, ValueType type);
	SceneValue channelValue(t_channel ch);
	
	ValueType valueType(t_channel ch);
	QString valueTypeString(t_channel ch);
	static ValueType stringToValueType(QString type);

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
	SceneValue* m_values;
	
	t_bus_value m_timeSpan;
	t_bus_value m_elapsedTime;
	
	RunTimeData* m_runTimeData;
	t_buffer_data* m_channelData;

	bool m_stopped;
};

class RunTimeData
{
public:
	float start;
	float current;
	float target;
	
	bool ready;
};

class SceneValue
{
public:
	Scene::ValueType type;
	t_value value;
};

#endif
