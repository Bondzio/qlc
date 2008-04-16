/*
  Q Light Controller
  advancedsceneeditor.h

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

#ifndef ADVANCEDSCENEEDITOR_H
#define ADVANCEDSCENEEDITOR_H

#include <QDialog>
#include "ui_advancedsceneeditor.cpp"
#include "function.h"

class QTreeWidgetItem;
class QPoint;

class Scene;
class QLCChannel;
class QLCFixtureDef;

typedef QList <t_channel> ChannelList;

class AdvancedSceneEditor : public QDialog, public Ui_AdvancedSceneEditor
{
	Q_OBJECT
    
	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	AdvancedSceneEditor(QWidget* parent, Scene* scene);
	virtual ~AdvancedSceneEditor();

protected:
	Scene* m_scene;
	Scene* m_original;

	/*********************************************************************
	 * Menu
	 *********************************************************************/
protected slots:
	void slotContextMenu(const QPoint &point);

	void slotCapabilityMenuActivated(int);
	void slotValueMenuActivated(int);
	void slotTypeMenuActivated(int);

protected:
	void invokeCapabilityMenu(const QPoint &);
	void invokeValueMenu(const QPoint &);
	void invokeTypeMenu(const QPoint &);

	/*********************************************************************
	 * Listview
	 *********************************************************************/
protected:
	/** Clear the list and create its contents anew */
	void initListView();

	/** Update only selected list view items (after an edit operation) */
	void updateSelectedItems();

	/** Update the given list view item to reflect the given channel value
	    in the currently edited scene */
	void updateChannelItem(QTreeWidgetItem* item, t_channel ch);

protected slots:
	void slotItemSelectionChanged();
	void slotDoubleClicked(QTreeWidgetItem*);

protected:
	/** List of selected channel numbers */
	ChannelList m_selection;

	/*********************************************************************
	 * Control buttons
	 *********************************************************************/
protected slots:
	void slotEditValueClicked();
};

#endif
