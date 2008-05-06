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
#include <iostream>
#include <QLayout>
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
	QPalette pal;

	m_fixture = KNoID;
	m_tempScene = NULL;

	setupUi(this);

	layout()->setContentsMargins(1, 1, 1, 1);

	/* Actions and tools menu */
	initActions();
	initMenu();

	/* Tools menu button */
	m_tools->setPopupMode(QToolButton::InstantPopup);
	m_tools->setIcon(QIcon(PIXMAPS "/scene.png"));
	m_tools->setText(tr("Scene editor"));
	m_tools->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

	/* Scene list */
	m_sceneList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_sceneList->setRootIsDecorated(false);
	m_sceneList->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(m_sceneList, SIGNAL(customContextMenuRequested(const QPoint&)),
		this, SLOT(slotSceneListContextMenu(const QPoint&)));
	connect(m_sceneList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(slotActivate()));
	connect(m_sceneList, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotItemSelectionChanged()));

	/* Status label */
	pal.setColor(QPalette::Window, QColor(0, 0, 0));
	m_statusLabel->setPalette(pal);
	m_statusLabel->setAutoFillBackground(true);
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
		item->setIcon(KColumnName, QIcon(PIXMAPS "/scene.png"));
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
		this, SLOT(slotActivate()));

	m_newAction = new QAction(QIcon(PIXMAPS "/wizard.png"),
				  tr("New scene..."), this);
	connect(m_newAction, SIGNAL(triggered(bool)),
		this, SLOT(slotNew()));

	m_storeAction = new QAction(QIcon(PIXMAPS "/filesave.png"),
				   tr("Store"), this);
	connect(m_storeAction, SIGNAL(triggered(bool)),
		this, SLOT(slotStore()));

	m_removeAction = new QAction(QIcon(PIXMAPS "/editdelete.png"),
				     tr("Remove"), this);
	connect(m_removeAction, SIGNAL(triggered(bool)),
		this, SLOT(slotRemove()));

	m_renameAction = new QAction(QIcon(PIXMAPS "/editclear.png"),
				     tr("Rename..."), this);
	connect(m_renameAction, SIGNAL(triggered(bool)),
		this, SLOT(slotRename()));

	/* Update action statuses */
	slotItemSelectionChanged();
}

void SceneEditor::initMenu()
{
	m_menu = new QMenu(m_tools);
	m_menu->setTitle(tr("Scene editor"));
	m_menu->addAction(m_activateAction);
	m_menu->addSeparator();
	m_menu->addAction(m_newAction);
	m_menu->addAction(m_storeAction);
	m_menu->addAction(m_renameAction);
	m_menu->addAction(m_removeAction);

	m_tools->setMenu(m_menu);
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

bool SceneEditor::slotNew()
{
	bool ok = false;
	QString name;

	name = QInputDialog::getText(this, tr("New scene"), tr("Scene name:"),
				     QLineEdit::Normal, QString::null, &ok);

	if (ok == true && name.isEmpty() == false)
	{
		Scene* sc = static_cast<Scene*>
			(_app->doc()->newFunction(Function::Scene,
						  m_tempScene->fixture()));

		sc->copyFrom(m_tempScene, m_tempScene->fixture());
		sc->setName(name);

		m_sceneList->sortItems(KColumnName, Qt::AscendingOrder);
		selectFunction(sc->id());

		setStatusText(KStatusStored, KStatusColorStored);
	}

	return ok;
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
			item->setIcon(KColumnName, QIcon(PIXMAPS "/scene.png"));
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
	while (*it != NULL)
	{
		if ((*it)->text(KColumnID).toInt() == fid)
		{
			m_sceneList->setCurrentItem(*it);
			m_sceneList->scrollToItem(*it);
			break;
		}
		++it;
	}
}

void SceneEditor::slotSceneListContextMenu(const QPoint &point)
{
	m_menu->exec(QCursor::pos());
}

void SceneEditor::slotItemSelectionChanged()
{
	if (m_sceneList->currentItem() == NULL)
	{
		m_activateAction->setEnabled(false);
		m_storeAction->setEnabled(false);
		m_renameAction->setEnabled(false);
		m_removeAction->setEnabled(false);
	}
	else
	{
		m_activateAction->setEnabled(true);
		m_storeAction->setEnabled(true);
		m_renameAction->setEnabled(true);
		m_removeAction->setEnabled(true);
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
