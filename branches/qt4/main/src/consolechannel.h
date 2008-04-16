/*
  Q Light Controller
  consolechannel.h
  
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

#ifndef CONSOLECHANNEL_H
#define CONSOLECHANNEL_H

#include <QWidget>
#include "ui_consolechannel.cpp"

#include "common/qlctypes.h"
#include "scene.h"

class QContextMenuEvent;
class QMenu;

class QLCChannel;
class Fixture;

class ConsoleChannel : public QWidget, public Ui_ConsoleChannel
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	ConsoleChannel(QWidget *parent, t_fixture_id fixtureID,
		       t_channel channel);
	~ConsoleChannel();

protected:
	/** Initialize the UI */
	void init();
 
public slots:
	/** Set channel's focus */
	void slotSetFocus();

	/**
	 * Fixture console's scene editor has activated a scene, which is
	 * reflected here. Set the value and status to this channel's widgets.
	 */
	void slotSceneActivated(SceneValue* values, t_channel channels);

	/*********************************************************************
	 * Menu
	 *********************************************************************/
protected slots:
	void slotContextMenuTriggered(QAction* action);

protected:
	/** Open a context menu */
	void contextMenuEvent(QContextMenuEvent*);

	/** Initialize the context menu */
	void initMenu();

	/** Initialize the context menu for plain dimmer fixtures */
	void initPlainMenu();

	/** Initialize the context menu for fixtures with capabilities */
	void initCapabilityMenu(QLCChannel* ch);

protected:
	QMenu* m_menu;

	/*********************************************************************
	 * Value
	 *********************************************************************/
public:
	/** Get the channel's value */
	int sliderValue() const;

	/** Update the UI to match the channel's real status & value */
	void update();

public slots:
	/** Slider value was changed */
	void slotValueChange(int);

	/** Emulate the user dragging the value slider */
	void slotAnimateValueChange(t_value);

protected:
	t_value m_value;

	/*********************************************************************
	 * Status
	 *********************************************************************/
public:
	/** Set status button's status */
	void setStatusButton(Scene::ValueType);

	Scene::ValueType status() const { return m_status; }

protected slots:
	/** Status button was clicked by the user */
	void slotStatusButtonClicked();

protected:
	/** Update the button's color, label and tip to show channel status */
	void updateStatusButton();

protected:
	Scene::ValueType m_status;

	/*********************************************************************
	 * Fixture channel
	 *********************************************************************/
protected:
	t_channel m_channel;
	t_fixture_id m_fixtureID;

signals:
	void changed(t_channel channel, t_value value, Scene::ValueType status);
};

#endif
