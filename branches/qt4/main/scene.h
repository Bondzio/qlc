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
#include <QtXml>

#include "common/qlctypes.h"

#include "eventbuffer.h"
#include "function.h"
#include "fixture.h"

#define KXMLQLCSceneValue "Value"
#define KXMLQLCSceneValueFixture "Fixture"
#define KXMLQLCSceneValueChannel "Channel"

/*****************************************************************************
 * SceneChannel
 *****************************************************************************/

class SceneChannel
{
public:
	/** The universe and channel that this object refers to */
	t_channel address;

	/** The value of the channel where a scene started fading from */
	float start;

	/** The current value set by a scene */
	float current;

	/** The target value to eventually fade to */
	float target;

	/** If true, this value is ready, don't set it anymore to DMX */
	bool ready;
};

/*****************************************************************************
 * SceneValue
 *****************************************************************************/

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
	/**
	 * Construct a new scene function, with parent object (Doc)
	 */
	Scene(QObject* parent);
	
	/**
	 * Destroy the scene
	 */
	~Scene();

	/**
	 * Copy all contents (except ID) from another scene function
	 */
	void copyFrom(Scene* scene);
	
	/*********************************************************************
	 * Values
	 *********************************************************************/
public:
	/**
	 * Set the value of one fixture channel, using a predefined SceneValue
	 */
	void setValue(SceneValue scv);

	/**
	 * Set the value of one fixture channel, specify parameters separately
	 */
	void setValue(t_fixture_id fxi, t_channel ch, t_value value);

	/**
	 * Clear the value of one fixture channel
	 */
	void unsetValue(t_fixture_id fxi, t_channel ch);

	/**
	 * Get the value of one fixture channel
	 */
	t_value value(t_fixture_id fxi, t_channel ch);

	/**
	 * Write the scene values to DMX Map. If fxi_id is given, writes
	 * values only for the specified fixture. 
	 */
	void writeValues(t_fixture_id fxi_id = KNoID);

	/**
	 * Get a list of values in this scene
	 */
	QList <SceneValue> *values() { return &m_values; }

protected:
	QList <SceneValue> m_values;	

	/*********************************************************************
	 * Fixtures
	 *********************************************************************/
public slots:
	void slotFixtureRemoved(t_fixture_id fxi_id);

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
	/**
	 * Listener for bus value changes
	 */
	void slotBusValueChanged(t_bus_id id, t_bus_value value);

public:
	/**
	 * Initiate speed change with the given bus value
	 */
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
	t_bus_value m_timeSpan;
	t_bus_value m_elapsedTime;

	SceneChannel* m_channels;
	t_buffer_data* m_channelData;

	bool m_stopped;
};

#endif
