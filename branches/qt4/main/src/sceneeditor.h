/*
  Q Light Controller
  sceneeditor.h

  Copyright (c) Heikki Junnila, Stefan Krumm

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

#ifndef SCENEEDITOR_H
#define SCENEEDITOR_H

#include <QWidget>

#include "ui_sceneeditor.cpp"
#include "common/qlctypes.h"
#include "scene.h"

class QAction;
class QMenu;

class SceneEditor : public QWidget, public Ui_SceneEditor
{
	Q_OBJECT

public:
	SceneEditor(QWidget* parent);
	~SceneEditor();

	/*********************************************************************
	 * Fixture
	 *********************************************************************/
public:
	void setFixture(t_fixture_id id);

protected:
	t_fixture_id m_fixture;

public slots:
	void slotChannelChanged(t_channel, t_value, Scene::ValueType);

	/** Signal handler for Doc::functionAdded signal */
	void slotFunctionAdded(t_function_id fid);

	/** Signal handler for Doc::functionRemoved signal */
	void slotFunctionRemoved(t_function_id fid);

	/** Signal handler for Doc::functionChanged signal */
	void slotFunctionChanged(t_function_id fid);

	/*********************************************************************
	 * Menu & Actions
	 *********************************************************************/
protected:
	void initActions();
	void initMenu();

protected slots:
	void slotActivate();
	bool slotNew();
	void slotStore();
	void slotRename();
	void slotRemove();

protected:
	QAction* m_activateAction;
	QAction* m_newAction;
	QAction* m_storeAction;
	QAction* m_renameAction;
	QAction* m_removeAction;

	QMenu* m_menu;

	/*********************************************************************
	 * Scene list
	 *********************************************************************/
protected:
	/** Fill the function list */
	void fillFunctions();

	/** Get the list item that has the given function ID */
	QTreeWidgetItem* getItem(t_function_id id);

	/** Select the item that has the given function ID */
	void selectFunction(t_function_id fid);

protected slots:
	/** Item selection changed */
	void slotItemSelectionChanged();

	/** Context menu has been requested for the scene list */
	void slotSceneListContextMenu(const QPoint& pos);

	/*********************************************************************
	 * Current scene
	 *********************************************************************/
protected:
	/** Get the current scene */
	Scene* currentScene();

signals:
	void sceneActivated(SceneValue* values, t_channel channels);

protected:
	Scene* m_tempScene;

	/*********************************************************************
	 * Status
	 *********************************************************************/
protected:
	/** Set status label's text and color */
	void setStatusText(QString text, QColor color);
};

#endif
