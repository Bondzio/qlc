/*
  Q Light Controller
  sceneeditor.cpp

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

#include <QTreeWidgetItem>
#include <QInputDialog>
#include <QTreeWidget>
#include <QMessageBox>
#include <QToolButton>
#include <QLabel>
#include <QMenu>

#include "common/qlcfixturedef.h"

#include "consolechannel.h"
#include "sceneeditor.h"
#include "function.h"
#include "fixture.h"
#include "scene.h"
#include "app.h"
#include "doc.h"

using namespace std;

extern App* _app;

#define KStatusStored "Stored"
#define KStatusUnchanged "Unchanged"
#define KStatusModified "Modified"

#define KStatusColorStored QColor(100, 255, 100)
#define KStatusColorUnchanged QColor(255, 255, 255)
#define KStatusColorModified QColor(255, 100, 100)

#define KColumnName 0
#define KColumnID   1

SceneEditor::SceneEditor(QWidget* parent) : QWidget(parent)
{
	m_fixture = KNoID;
	m_tempScene = NULL;

	setupUi(this);

	m_sceneList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	connect(m_sceneList,
		SIGNAL(customContextMenuRequested(const QPoint&)),
		this,
		SLOT(slotSceneListContextMenu(const QPoint&)));

	initActions();
	initMenu();
}

SceneEditor::~SceneEditor()
{
	if (m_tempScene != NULL)
		delete m_tempScene;
	m_tempScene = NULL;
}

void SceneEditor::setFixture(t_fixture_id id)
{
	m_fixture = id;

	// The scene that contains all the edited values
	if (m_tempScene)
		delete m_tempScene;
	m_tempScene = new Scene();
	m_tempScene->setFixture(id);

	fillFunctions();
}

void SceneEditor::slotChannelChanged(t_channel channel, t_value value,
				     Scene::ValueType status)
{
	Q_ASSERT(m_tempScene != NULL);
	m_tempScene->set(channel, value, status);

	setStatusText(KStatusModified, KStatusColorModified);
}

void SceneEditor::slotFunctionAdded(t_function_id id)
{	
	QTreeWidgetItem* item;
	Function* function;

	function = _app->doc()->function(id);
	Q_ASSERT(function != NULL);

	item = getItem(id);

	// We are interested only in scenes that are members of this
	// console's fixture
	if (item == NULL && function->type() == Function::Scene &&
	    function->fixture() == m_fixture)
	{
		item = new QTreeWidgetItem(m_sceneList);
		item->setText(KColumnName, function->name());
		item->setText(KColumnID, QString("%1").arg(id));
	}
}

void SceneEditor::slotFunctionRemoved(t_function_id id)
{
	QTreeWidgetItem* nextItem;
	QTreeWidgetItem* item;

	item = getItem(id);
	if (item != NULL)
	{
		if (item->isSelected() == true)
		{
			// Select an item below or above if the current item
			// was removed.
			if (m_sceneList->itemBelow(item) != NULL)
				nextItem = m_sceneList->itemBelow(item);
			else
				nextItem = m_sceneList->itemAbove(item);

			if (nextItem != NULL)
				nextItem->setSelected(true);
		}
      
		delete item;
	}
}

void SceneEditor::slotFunctionChanged(t_function_id id)
{
	QTreeWidgetItem* item;
	item = getItem(id);
	if (item != NULL)
	{
		Function* function = _app->doc()->function(id);
		if (function != NULL && function->type() == Function::Scene)
		{
			item->setText(KColumnName, function->name());
			item->setText(KColumnID,
				      QString("%1").arg(function->id()));
		}
	}
}

/*****************************************************************************
 * Menu & Actions
 *****************************************************************************/

void SceneEditor::initActions()
{
	m_activateAction = new QAction(QIcon(PIXMAPS "/apply.png"),
				       tr("Activate"), this);
	connect(m_activateAction, SIGNAL(triggered(bool)),
		this, SLOT(slotActivateTriggered()));

	m_newAction = new QAction(QIcon(PIXMAPS "/wizard.png"),
				  tr("New scene..."), this);
	connect(m_newAction, SIGNAL(triggered(bool)),
		this, SLOT(slotNewTriggered()));

	m_storeAction = new QAction(QIcon(PIXMAPS "/filesave.png"),
				   tr("Store"), this);
	connect(m_storeAction, SIGNAL(triggered(bool)),
		this, SLOT(slotSaveTriggered()));

	m_removeAction = new QAction(QIcon(PIXMAPS "editdelete.png"),
				     tr("Remove"), this);
	connect(m_removeAction, SIGNAL(triggered(bool)),
		this, SLOT(slotRemoveTriggered()));

	m_renameAction = new QAction(QIcon(PIXMAPS "editclear.png"),
				     tr("Rename..."), this);
	connect(m_renameAction, SIGNAL(triggered(bool)),
		this, SLOT(slotRenameTriggered()));
}

void SceneEditor::initMenu()
{
	m_menu = new QMenu(m_tools);
	m_menu->addAction(m_activateAction);
	m_menu->addAction(m_storeAction);
	m_menu->addSeparator();
	m_menu->addAction(m_newAction);
	m_menu->addAction(m_renameAction);
	m_menu->addSeparator();
	m_menu->addAction(m_removeAction);

	m_tools->setMenu(m_menu);
	m_tools->setIcon(QIcon(PIXMAPS "/scene.png"));
}

void SceneEditor::slotSceneListContextMenu(const QPoint &point)
{
	m_menu->exec(point);
}

void SceneEditor::slotActivate()
{
	Scene* scene = currentScene();

	if (scene != NULL)
	{
		m_tempScene->copyFrom(scene, scene->fixture());
		emit sceneActivated(scene->values(), scene->channels());
	}

	setStatusText(KStatusUnchanged, KStatusColorUnchanged);
}

void SceneEditor::slotNew()
{
	bool ok = false;
	QString name;
	name = QInputDialog::getText(this, tr("New scene"),
				     tr("Scene name:"), QLineEdit::Normal,
				     QString::null, &ok);

	if (ok == true && name.isEmpty() == false)
	{
		Scene* sc = static_cast<Scene*>
			(_app->doc()->newFunction(Function::Scene,
						  m_tempScene->fixture()));

		sc->copyFrom(m_tempScene, m_tempScene->fixture());
		sc->setName(name);

		m_sceneList->sortItems(KColumnName, Qt::AscendingOrder);
		selectFunction(sc->id());
		m_sceneList->scrollToItem(getItem(sc->id()));

		setStatusText(KStatusStored, KStatusColorStored);
	}
}

void SceneEditor::slotStore()
{
	Scene* sc = currentScene();
	if (sc == NULL)
		return;

	// Save name & bus because copyFrom overwrites them
	QString name = sc->name();
	t_bus_id bus = sc->busID();

	sc->copyFrom(m_tempScene, m_tempScene->fixture());

	// Set these again
	sc->setName(name);
	sc->setBus(bus);

	setStatusText(KStatusStored, KStatusColorStored);
}

void SceneEditor::slotRename()
{
	bool ok = false;
	Scene* scene;
	QString name;

	scene = currentScene();
	if (scene == NULL)
		return;

	name = QInputDialog::getText(this, tr("Rename Scene"),
				     tr("Scene name:"), QLineEdit::Normal,
				     scene->name(), &ok);
	if (ok == true && name.isEmpty() == false)
	{
		scene->setName(name);
		fillFunctions();
		selectFunction(scene->id());
	}
}

void SceneEditor::slotRemove()
{
	Scene* scene = currentScene();

	if (scene == NULL)
		return;

	if (QMessageBox::question(this, tr("Remove function"),
				  QString("Do you want to remove \"%1\"?")
				  .arg(scene->name()),
				  QMessageBox::Yes, QMessageBox::No)
	    == QMessageBox::Yes)
	{
		_app->doc()->deleteFunction(scene->id());
		fillFunctions();
	}
}

/*****************************************************************************
 * Scene list
 *****************************************************************************/

void SceneEditor::fillFunctions()
{
	QTreeWidgetItem* item;

	m_sceneList->clear();

	for (t_function_id id = 0; id < KFunctionArraySize; id++)
	{
		Function* function = _app->doc()->function(id);
		if (function == NULL)
			continue;
		
		if (function->type() == Function::Scene &&
		    function->fixture() == m_fixture)
		{
			QString str;
			item = new QTreeWidgetItem(m_sceneList);
			item->setText(KColumnName, function->name());
			item->setText(KColumnID, str.setNum(id));
		}
	}

	m_sceneList->sortItems(KColumnName, Qt::AscendingOrder);

	setStatusText(KStatusUnchanged, KStatusColorUnchanged);
}

QTreeWidgetItem* SceneEditor::getItem(t_function_id id)
{
	QTreeWidgetItemIterator it(m_sceneList);
	while (*it != NULL)
	{
		if ((*it)->text(KColumnID).toInt() == id)
			return *it;
		++it;
	}
  
	return NULL;
}

void SceneEditor::selectFunction(t_function_id fid)
{
	QTreeWidgetItemIterator it(m_sceneList);
	while (*it)
	{
		if ((*it)->text(KColumnID).toInt() == fid)
		{
			(*it)->setSelected(true);
			m_sceneList->scrollToItem(*it);
			break;
		}
	}
}

/*****************************************************************************
 * Current scene
 *****************************************************************************/

Scene* SceneEditor::currentScene()
{
	QTreeWidgetItem* item;
	t_function_id fid;

	item = m_sceneList->currentItem();
	if (item == NULL)
		return NULL;
	
	fid = item->text(KColumnID).toInt();
	return static_cast<Scene*> (_app->doc()->function(fid));
}

/*****************************************************************************
 * Status label
 *****************************************************************************/

void SceneEditor::setStatusText(QString text, QColor color)
{
	QPalette p = m_statusLabel->palette();
	p.setColor(QPalette::Window, color);
	m_statusLabel->setPalette(p);

	m_statusLabel->setText(text);
}
