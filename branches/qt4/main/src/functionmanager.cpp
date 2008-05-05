/*
  Q Light Controller
  functionmanager.cpp

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

#include <QTreeWidgetItemIterator>
#include <QTreeWidgetItem>
#include <QInputDialog>
#include <QTreeWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QSplitter>
#include <QToolBar>
#include <QMenuBar>
#include <iostream>
#include <QPixmap>
#include <QMenu>
#include <QList>

#include "common/qlcfixturedef.h"

#include "collectioneditor.h"
#include "functionmanager.h"
#include "chasereditor.h"
#include "collection.h"
#include "efxeditor.h"
#include "function.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "app.h"
#include "doc.h"
#include "efx.h"

#define KColumnName 0
#define KColumnBus  1
#define KColumnID   2

using namespace std;

extern App* _app;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

FunctionManager::FunctionManager(QWidget* parent) : QWidget(parent)
{
	m_clipboardAction = ClipboardNone;
	
	m_blockAddFunctionSignal = false;
	m_blockRemoveFunctionSignal = false;

	new QVBoxLayout(this);

	initActions();
	initMenu();
	initToolbar();

	// Create the splitter which contains the fixture & function trees
	m_splitter = new QSplitter(this);
	layout()->addWidget(m_splitter);
	m_splitter->setSizePolicy(QSizePolicy::Expanding,
				  QSizePolicy::Expanding);

	// Create fixture tree
	initFixtureTree();

	// Create function tree
	initFunctionTree();

	// Clear clipboard contents
	m_clipboard.clear();

	// Select the first fixture
	m_fixtureTree->setCurrentItem(m_fixtureTree->topLevelItem(0));
}

FunctionManager::~FunctionManager()
{
}

/*****************************************************************************
 * Doc signal handlers
 *****************************************************************************/

void FunctionManager::slotFixtureAdded(t_fixture_id id)
{
	QTreeWidgetItem* item;
	Fixture* fxi;
	QString s;

	fxi = _app->doc()->fixture(id);
	if (fxi != NULL)
	{
		item = new QTreeWidgetItem(m_fixtureTree);
		item->setIcon(KColumnName, QIcon(PIXMAPS "/fixture.png"));
		item->setText(KColumnName, fxi->name());
		item->setText(KColumnID, s.setNum(id));
	}
}

void FunctionManager::slotFixtureRemoved(t_fixture_id id)
{
	QTreeWidgetItem* item;

	item = getItem(id, m_fixtureTree);
	if (item != NULL)
	{
		if (item->isSelected() == true)
		{
			QTreeWidgetItem* nextItem;

			// Try to select the closest neighbour
			if (m_fixtureTree->itemAbove(item) != NULL)
				nextItem = m_fixtureTree->itemAbove(item);
			else
				nextItem = m_fixtureTree->itemBelow(item);

			if (nextItem != NULL)
				nextItem->setSelected(true);
		}

		delete item;
	}
}

void FunctionManager::slotFixtureChanged(t_fixture_id id)
{
	QTreeWidgetItem* item;

	item = getItem(id, m_fixtureTree);
	if (item != NULL)
	{
		Fixture* fxi = _app->doc()->fixture(id);
		Q_ASSERT(fxi != NULL);

		item->setText(KColumnName, fxi->name());
	}
}

void FunctionManager::slotFunctionAdded(t_function_id id)
{
	QTreeWidgetItem* fixtureItem;
	QTreeWidgetItem* item;
	Function* function;

	// The function manager has its own routines for functions that are
	// created with it.
	if (m_blockAddFunctionSignal == true)
		return;
	
	fixtureItem = m_fixtureTree->currentItem();
	if (fixtureItem == NULL)
		return; // No fixture, so function tree is empty, nothing to do

	function = _app->doc()->function(id);
	if (function != NULL)
	{
		// Check if the selected fixture is the one that the newly
		// added function belongs to.
		if (fixtureItem->text(KColumnID).toInt() == function->fixture())
		{
			// Create a new item for the function
			item = new QTreeWidgetItem(m_functionTree);
			updateFunctionItem(item, function);
		}
	}
}

void FunctionManager::slotFunctionRemoved(t_function_id id)
{
	QTreeWidgetItem* item;

	// The function manager has its own routines for functions that are
	// removed with it.
	if (m_blockRemoveFunctionSignal == true)
		return;

	item = getItem(id, m_functionTree);
	if (item != NULL)
	{
		if (item->isSelected() == true)
		{
			QTreeWidgetItem* nextItem;

			// Try to select the closest neighbour
			if (m_fixtureTree->itemAbove(item) != NULL)
				nextItem = m_fixtureTree->itemAbove(item);
			else
				nextItem = m_fixtureTree->itemBelow(item);
	  
			if (nextItem != NULL)
				nextItem->setSelected(true);
		}
      
		delete item;
	}
}

void FunctionManager::slotFunctionChanged(t_function_id id)
{
	QTreeWidgetItem* item;

	item = getItem(id, m_functionTree);
	if (item != NULL)
	{
		Function* function = _app->doc()->function(id);
		updateFunctionItem(item, function);
	}
}

/*****************************************************************************
 * Menu, toolbar and actions
 *****************************************************************************/

void FunctionManager::initActions()
{
	Q_ASSERT(parentWidget() != NULL);

	/* Manage actions */
	m_addSceneAction = new QAction(QIcon(PIXMAPS "/scene.png"),
				       tr("New scene"), this);
	connect(m_addSceneAction, SIGNAL(triggered(bool)),
		this, SLOT(slotAddScene()));

	m_addChaserAction = new QAction(QIcon(PIXMAPS "/chaser.png"),
					tr("New chaser"), this);
	connect(m_addChaserAction, SIGNAL(triggered(bool)),
		this, SLOT(slotAddChaser()));

	m_addCollectionAction = new QAction(QIcon(PIXMAPS "/collection.png"),
					    tr("New collection"), this);
	connect(m_addCollectionAction, SIGNAL(triggered(bool)),
		this, SLOT(slotAddCollection()));

	m_addEFXAction = new QAction(QIcon(PIXMAPS "/efx.png"),
				     tr("New EFX"), this);
	connect(m_addEFXAction, SIGNAL(triggered(bool)),
		this, SLOT(slotAddEFX()));

	m_closeAction = new QAction(QIcon(PIXMAPS "/fileclose.png"),
				    tr("Close"), this);
	connect(m_closeAction, SIGNAL(triggered(bool)),
		parentWidget(), SLOT(close()));

	/* Edit actions */
	m_editAction = new QAction(QIcon(PIXMAPS "/edit.png"),
				   tr("Edit"), this);
	connect(m_editAction, SIGNAL(triggered(bool)),
		this, SLOT(slotEdit()));

	m_cutAction = new QAction(QIcon(PIXMAPS "/editcut.png"),
				  tr("Cut"), this);
	connect(m_cutAction, SIGNAL(triggered(bool)),
		this, SLOT(slotCut()));

	m_copyAction = new QAction(QIcon(PIXMAPS "/editcopy.png"),
				   tr("Copy"), this);
	connect(m_copyAction, SIGNAL(triggered(bool)),
		this, SLOT(slotCopy()));

	m_pasteAction = new QAction(QIcon(PIXMAPS "/editpaste.png"),
				    tr("Paste"), this);
	connect(m_pasteAction, SIGNAL(triggered(bool)),
		this, SLOT(slotPaste()));

	m_deleteAction = new QAction(QIcon(PIXMAPS "/editdelete.png"),
				     tr("Delete"), this);
	connect(m_deleteAction, SIGNAL(triggered(bool)),
		this, SLOT(slotDelete()));

	m_selectAllAction = new QAction(QIcon(PIXMAPS "/selectall.png"),
					tr("Select all"), this);
	connect(m_selectAllAction, SIGNAL(triggered(bool)),
		this, SLOT(slotSelectAll()));
}

void FunctionManager::initMenu()
{
	QAction* action;

	layout()->setMenuBar(new QMenuBar(this));

	/* Function menu */
	m_manageMenu = new QMenu(this);
	m_manageMenu->setTitle(tr("Manage"));
	m_manageMenu->addAction(m_addSceneAction);
	m_manageMenu->addAction(m_addChaserAction);
	m_manageMenu->addAction(m_addCollectionAction);
	m_manageMenu->addAction(m_addEFXAction);
	m_manageMenu->addSeparator();
	m_manageMenu->addAction(m_closeAction);

	/* Edit menu */
	m_editMenu = new QMenu(this);
	m_editMenu->setTitle("Edit");
	m_editMenu->addAction(m_editAction);
	m_editMenu->addSeparator();
	m_editMenu->addAction(m_cutAction);
	m_editMenu->addAction(m_copyAction);
	m_editMenu->addAction(m_pasteAction);
	m_editMenu->addSeparator();
	m_editMenu->addAction(m_deleteAction);
	m_editMenu->addAction(m_selectAllAction);
	m_editMenu->addSeparator();

	/* Bus menu */
	m_busGroup = new QActionGroup(this);
	m_busMenu = new QMenu(this);
	m_busMenu->setTitle("Bus");
	for (t_bus_id id = KBusIDMin; id < KBusCount; id++)
	{
		/* <xx>: <name> */
		action = new QAction(QString("%1: %2").arg(id + 1, 2, 10,
							   QChar('0'))
				     .arg(Bus::name(id)), this);
		action->setCheckable(true);
		m_busGroup->addAction(action);
		m_busActions.append(action);
		m_busMenu->addAction(action);
	}

	/* Catch bus assignment changes */
	connect(m_busGroup, SIGNAL(triggered(QAction*)),
		this, SLOT(slotBusTriggered(QAction*)));

	/* Catch bus name changes */
	connect(Bus::emitter(), SIGNAL(nameChanged(t_bus_id, const QString&)),
		this, SLOT(slotBusNameChanged(t_bus_id, const QString&)));

	/* Construct menu bar */
	static_cast<QMenuBar*>(layout()->menuBar())->addMenu(m_manageMenu);
	static_cast<QMenuBar*>(layout()->menuBar())->addMenu(m_editMenu);
	m_editMenu->addMenu(m_busMenu);
}

void FunctionManager::initToolbar()
{
	// Add a toolbar to the dock area
	m_toolbar = new QToolBar("Function Manager", this);
	layout()->addWidget(m_toolbar);
	m_toolbar->addAction(m_addSceneAction);
	m_toolbar->addAction(m_addChaserAction);
	m_toolbar->addAction(m_addCollectionAction);
	m_toolbar->addAction(m_addEFXAction);
	m_toolbar->addSeparator();
	m_toolbar->addAction(m_editAction);
	m_toolbar->addSeparator();
	m_toolbar->addAction(m_cutAction);
	m_toolbar->addAction(m_copyAction);
	m_toolbar->addAction(m_pasteAction);
	m_toolbar->addSeparator();
	m_toolbar->addAction(m_deleteAction);
}

void FunctionManager::slotBusTriggered(QAction* action)
{
	/* Bus actions are in ascending order in the list so the index of
	   the triggered action is also the ID of the bus represented by
	   the action. */
	t_bus_id busID = m_busActions.indexOf(action);

	/* Set the selected bus to all selected functions */
	QListIterator <QTreeWidgetItem*> it(m_functionTree->selectedItems());
	while (it.hasNext() == true)
	{
		QTreeWidgetItem* item;
		Function* function;

		item = it.next();
		Q_ASSERT(item != NULL);
		function = _app->doc()->function(item->text(KColumnID).toInt());
		Q_ASSERT(function != NULL);

		function->setBus(busID);
		updateFunctionItem(item, function);
	}
}

void FunctionManager::slotBusNameChanged(t_bus_id id, const QString& name)
{
	QAction* action;

	action = m_busActions.at(id);
	Q_ASSERT(action != NULL);

	action->setText(QString("%1: %2").arg(id).arg(name));
}

void FunctionManager::slotAddScene()
{
	addFunction(Function::Scene);
}

void FunctionManager::slotAddChaser()
{
	addFunction(Function::Chaser);
}

void FunctionManager::slotAddCollection()
{
	addFunction(Function::Collection);
}

void FunctionManager::slotAddEFX()
{
	addFunction(Function::EFX);
}

int FunctionManager::slotEdit()
{
	QTreeWidgetItem* item;
	Function* function;
	int result;

	item = m_functionTree->selectedItems().first();
	if (item == NULL)
		return QDialog::Rejected;

	// Find the selected function
	function = _app->doc()->function(item->text(KColumnID).toInt());
	if (function == NULL)
		return QDialog::Rejected;

	// Edit the selected function
	switch (function->type())
	{
	case Function::Scene:
	{
		QMessageBox::information(this, "TODO", "TODO");
		//AdvancedSceneEditor ase(this, static_cast<Scene*> (function));
		//result = ase.exec();
	}
	break;

	case Function::Chaser:
	{
		ChaserEditor ce(this, static_cast<Chaser*> (function));
		result = ce.exec();
	}
	break;

	case Function::Collection:
	{
		CollectionEditor fce(this, static_cast<Collection*> (function));
		result = fce.exec();
	}
	break;

	case Function::EFX:
	{
		EFXEditor ee(this, static_cast<EFX*> (function));
		result = ee.exec();
	}
	break;

	default:
		result = QDialog::Rejected;
		break;
	}

	updateFunctionItem(item, function);

	return result;
}

void FunctionManager::slotCut()
{
	// Clear existing stuff from clipboard
	m_clipboard.clear();
	
	QListIterator <QTreeWidgetItem*> it(m_functionTree->selectedItems());
	while (it.hasNext() == true)
	{
		QTreeWidgetItem* item = it.next();

		// Add function ID to clipboard
		m_clipboard.append(item->text(KColumnID).toInt());

		// Set the action to Cut so that we know what to do with paste
		m_clipboardAction = ClipboardCut;

		// Set the item disabled to indicate it is about to get cut
		item->setDisabled(true);
	}
}

void FunctionManager::slotCopy()
{
	// Clear existing stuff from the clipboard
	m_clipboard.clear();

	QListIterator <QTreeWidgetItem*> it(m_functionTree->selectedItems());
	while (it.hasNext() == true)
	{
		QTreeWidgetItem* item = it.next();

		// Add selected function ID's to clipboard
		m_clipboard.append(item->text(KColumnID).toInt());

		// Set the action to Cut so that we know what to
		// do with paste
		m_clipboardAction = ClipboardCopy;

		// In case the user does a cut & then copy, enable the
		// selected items because cut disables them.
		item->setDisabled(false);
	}
}

void FunctionManager::slotPaste()
{
	t_fixture_id fxi_id;

	QTreeWidgetItem* fixtureitem = m_fixtureTree->currentItem();
	if (fixtureitem == NULL)
		return;

	if (m_clipboardAction == ClipboardCut)
	{
		// Get the currently selected fixture item
		fxi_id = fixtureitem->text(KColumnID).toInt();

		QListIterator <t_function_id> it(m_clipboard);
		while (it.hasNext() == true)
		{
			t_function_id fid = it.next();

			// Copy the function
			if (copyFunction(fid, fxi_id) == NULL)
			{
				// Stop pasting so we don't get the user
				// too annoyed with a message box for each
				// function. They all will fail, because
				// the user can't select functions from more
				// than one fixture (or global) at a time
				break;
			}
			else
			{
				// Delete the original function because we are
				// doing cut-paste
				_app->doc()->deleteFunction(fid);
			}
		}

		// Since the functions that were cut previously, are now
		// deleted, their function id's in the clipboard are also
		// invalid.
		m_clipboard.clear();
	}
	else if (m_clipboardAction == ClipboardCopy)
	{
		// Get the currently selected fixture item
		fxi_id = fixtureitem->text(KColumnID).toInt();

		QListIterator <t_function_id> it(m_clipboard);
		while (it.hasNext() == true)
		{
			// Copy the function
			if (copyFunction(it.next(), fxi_id) == NULL)
			{
				// Stop pasting so we don't get the user
				// too annoyed with a message box for each
				// function. They all will fail, because
				// the user can't select functions from more
				// than one fixture (or global) at a time
				break;
			}
		}
	}
}

void FunctionManager::slotDelete()
{
	QListIterator <QTreeWidgetItem*> it(m_functionTree->selectedItems());
	QString msg;

	if (it.hasNext() == false)
		return;

	msg = "Do you want to delete function(s):\n";

	// Append functions' names to the message
	while (it.hasNext() == true)
		msg += it.next()->text(KColumnName) + QString("\n");

	// Ask for user's confirmation
	if (QMessageBox::question(this, "Delete function(s)", msg,
				  QMessageBox::Yes, QMessageBox::No)
	    == QMessageBox::Yes)
	{
		deleteSelectedFunctions();
		updateActionStatus();
	}
}

void FunctionManager::slotSelectAll()
{
	for (int i = 0; i < m_functionTree->topLevelItemCount(); i++)
		m_functionTree->topLevelItem(i)->setSelected(true);
}

void FunctionManager::updateActionStatus()
{
	QTreeWidgetItem* fixtureItem = m_fixtureTree->currentItem();
	if (fixtureItem == NULL)
	{
		// There are no fixtures (at least none is selected)
		// so nothing can be done in function manager
		m_addSceneAction->setEnabled(false);
		m_addChaserAction->setEnabled(false);
		m_addCollectionAction->setEnabled(false);
		m_addEFXAction->setEnabled(false);

		m_editAction->setEnabled(false);
		m_cutAction->setEnabled(false);
		m_copyAction->setEnabled(false);
		m_pasteAction->setEnabled(false);
		m_deleteAction->setEnabled(false);
		m_selectAllAction->setEnabled(false);
	}
	else
	{
		// Get the selected fixture
		t_fixture_id fxi_id = fixtureItem->text(KColumnID).toInt();
		if (fxi_id == KNoID)
		{
			// Global fixture has been selected
			// Disable fixture-only functions
			m_addSceneAction->setEnabled(false);
			m_addChaserAction->setEnabled(true);
			m_addCollectionAction->setEnabled(true);
			m_addEFXAction->setEnabled(false);
		}
		else
		{
			// A regular fixture has been selected
			// Disable global-only functions
			m_addSceneAction->setEnabled(true);
			m_addChaserAction->setEnabled(false);
			m_addCollectionAction->setEnabled(false);
			m_addEFXAction->setEnabled(true);
		}

		if (m_functionTree->selectedItems().count() > 0)
		{
			/* At least one function has been selected, so
			   editing is possible. */
			m_editAction->setEnabled(true);
			m_cutAction->setEnabled(true);
			m_copyAction->setEnabled(true);

			m_deleteAction->setEnabled(true);
			m_selectAllAction->setEnabled(true);
		}
		else
		{
			/* No functions selected */
			m_editAction->setEnabled(false);
			m_cutAction->setEnabled(false);
			m_copyAction->setEnabled(false);

			m_deleteAction->setEnabled(false);
			m_selectAllAction->setEnabled(false);
		}

		/* Check, whether clipboard contains something to paste */
		if (m_clipboard.count() > 0)
		{
			m_pasteAction->setEnabled(true);
		}
		else
		{
			m_pasteAction->setEnabled(false);
			m_clipboardAction = ClipboardNone;
		}
	}

	/* Update bus menu actions in both cases */
	updateBusActions();
}

void FunctionManager::updateBusActions()
{
	QTreeWidgetItem* functionItem = m_functionTree->currentItem();
	if (functionItem == NULL)
	{
		/* No current function item selection, disable bus actions */
		m_busGroup->setEnabled(false);
	}
	else
	{
		Function* function;
		t_function_id fid;
		QAction* action;

		/* Find the selected function */
		fid = functionItem->text(KColumnID).toInt();
		function = _app->doc()->function(fid);
		Q_ASSERT(function != NULL);

		/* Collections don't have a bus ID and it's always KNoID.
		   Other functions' bus ID's can never be KNoID so it is
		   safe to check the function's bus-assignability this way */
		if (function->busID() == KNoID)
		{
			m_busGroup->setEnabled(false);
		}
		else
		{
			m_busGroup->setEnabled(true);

			/* Set the current function's bus checked */
			action = m_busActions.at(function->busID());
			Q_ASSERT(action != NULL);
			action->setChecked(true);
		}
	}
}

/****************************************************************************
 * Fixture tree
 ****************************************************************************/

void FunctionManager::initFixtureTree()
{
	// Create the tree view as the first child of the splitter
	m_fixtureTree = new QTreeWidget(m_splitter);
	m_splitter->addWidget(m_fixtureTree);

	// Add the one and only column
	m_fixtureTree->setHeaderLabel(tr("Fixture"));
	m_fixtureTree->header()->setResizeMode(QHeaderView::ResizeToContents);
	m_fixtureTree->setRootIsDecorated(false);
	m_fixtureTree->setAllColumnsShowFocus(true);
	m_fixtureTree->setSelectionMode(QAbstractItemView::SingleSelection);
	m_fixtureTree->setContextMenuPolicy(Qt::CustomContextMenu);

	// Catch selection changes
	connect(m_fixtureTree, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotFixtureTreeSelectionChanged()));

	// Catch right-mouse clicks
	connect(m_fixtureTree,
		SIGNAL(customContextMenuRequested(const QPoint&)),
		this,
		SLOT(slotFixtureTreeContextMenuRequested(const QPoint&)));

	updateFixtureTree();
}

void FunctionManager::updateFixtureTree()
{
	QString devid;

	m_fixtureTree->clear();

	//
	// Global root node
	//
	QTreeWidgetItem* item = new QTreeWidgetItem(m_fixtureTree);
	item->setText(KColumnName, "Global");
	item->setText(KColumnID, devid.setNum(KNoID));
	item->setIcon(KColumnName, QIcon(PIXMAPS "/global.png"));

	//
	// Fixture root nodes
	//
	for (t_fixture_id id = 0; id < KFixtureArraySize; id++)
	{
		Fixture* fxi = _app->doc()->fixture(id);
		if (fxi == NULL)
			continue;

		item = new QTreeWidgetItem(m_fixtureTree);
		item->setText(KColumnName, fxi->name());
		item->setText(KColumnID, devid.setNum(id));
		item->setIcon(KColumnName, QIcon(PIXMAPS "/fixture.png"));
	}
}

void FunctionManager::slotFixtureTreeSelectionChanged()
{
	QTreeWidgetItem* item;
	t_fixture_id fxi_id;

	item = m_fixtureTree->currentItem();
	if (item == NULL)
		return;

	// Get the selected fixture's ID
	fxi_id = item->text(KColumnID).toInt();

	// Clear the contents of the function tree
	m_functionTree->clear();

	// Get all functions belonging to the selected fixture
	for (t_function_id fid = 0; fid < KFunctionArraySize; fid++)
	{
		Function* function = _app->doc()->function(fid);
		if (function != NULL && function->fixture() == fxi_id)
		{
			item = new QTreeWidgetItem(m_functionTree);
			updateFunctionItem(item, function);
		}
	}

	updateActionStatus();
}

void FunctionManager::slotFixtureTreeContextMenuRequested(const QPoint& pos)
{
	QMenu contextMenu(this);
	contextMenu.setTitle("Fixture");
	contextMenu.addMenu(m_manageMenu);
	contextMenu.addMenu(m_editMenu);

	updateActionStatus();

	// No need to have cut/copy/delete/edit/select in fixture menu
	m_cutAction->setEnabled(false);
	m_copyAction->setEnabled(false);
	m_deleteAction->setEnabled(false);
	m_editAction->setEnabled(false);
	m_selectAllAction->setEnabled(false);

	contextMenu.exec(QCursor::pos());

	// Enable the menu items again
	updateActionStatus();
}

/****************************************************************************
 * Function tree
 ****************************************************************************/

void FunctionManager::initFunctionTree()
{
	// Create the tree view as the second child of the splitter
	m_functionTree = new QTreeWidget(m_splitter);
	m_splitter->addWidget(m_functionTree);

	// Add two columns for function and bus
	QStringList labels;
	labels << "Function" << "Bus";
	m_functionTree->setHeaderLabels(labels);
	m_functionTree->header()->setResizeMode(QHeaderView::ResizeToContents);
	m_functionTree->setRootIsDecorated(false);
	m_functionTree->setAllColumnsShowFocus(true);
	m_functionTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_functionTree->setContextMenuPolicy(Qt::CustomContextMenu);

	// Catch selection changes
	connect(m_functionTree, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotFunctionTreeSelectionChanged()));

	// Catch mouse double clicks
	connect(m_functionTree,	SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(slotEdit()));

	// Catch right-mouse clicks
	connect(m_functionTree,
		SIGNAL(customContextMenuRequested(const QPoint&)),
		this,
		SLOT(slotFunctionTreeContextMenuRequested(const QPoint&)));
}

void FunctionManager::updateFunctionItem(QTreeWidgetItem* item,
					 Function* function)
{
	if (item == NULL || function == NULL)
		return;

	item->setText(KColumnName, function->name());
	item->setIcon(KColumnName, function->pixmap());
	item->setText(KColumnBus, function->busName());
	item->setText(KColumnID, QString::number(function->id()));
}

void FunctionManager::deleteSelectedFunctions()
{
	m_blockRemoveFunctionSignal = true;
	QListIterator <QTreeWidgetItem*> it(m_functionTree->selectedItems());
	while (it.hasNext() == true)
	{
		QTreeWidgetItem* item;
		t_function_id fid;

		item = it.next();
		fid = item->text(KColumnID).toInt();
		_app->doc()->deleteFunction(fid);

		/* This is pretty weird, since QTreeWidget::selectedItems()
		   is const, but this seems to work so let's ship it... */
		delete item;
	}

	m_blockRemoveFunctionSignal = false;
}

void FunctionManager::slotFunctionTreeSelectionChanged()
{
	updateActionStatus();
}

void FunctionManager::slotFunctionTreeContextMenuRequested(const QPoint& pos)
{
	QMenu contextMenu(this);
	contextMenu.addMenu(m_manageMenu);
	contextMenu.addMenu(m_editMenu);

	updateActionStatus();

	contextMenu.exec(QCursor::pos());
}

/*****************************************************************************
 * Helpers
 *****************************************************************************/

Function* FunctionManager::copyFunction(t_function_id fid, t_fixture_id fxi_id)
{
	Function* newFunction = NULL;
	QString msg;

	Function* function = _app->doc()->function(fid);
	Q_ASSERT(function != NULL);

	// Check that we are not trying to copy global functions to a fixture
	// or vice versa
	if (fxi_id == KNoID)
	{
		if (function->type() == Function::Scene)
		{
			msg = "Scenes cannot be created to global space.\n";
			msg += "Paste aborted.\n";
		}
		else if (function->type() == Function::EFX)
		{
			msg = "EFX's cannot be created to global space.\n";
			msg += "Paste aborted.\n";
		}
		else
		{
			msg = QString::null;
		}
	}
	else if (fxi_id != KNoID)
	{
		if (function->type() == Function::Chaser)
		{
			msg = "Chasers cannot be created for fixtures.\n";
			msg += "Paste aborted.\n";
		}
		else if (function->type() == Function::Collection)
		{
			msg = "Collections cannot be created for fixtures.\n";
			msg += "Paste aborted.\n";
		}
		else
		{
			msg = QString::null;
		}
	}

	if (msg != QString::null)
	{
		// Display error message and exit without creating functions
		QMessageBox::warning(this, "Unable to paste functions", msg);
		return NULL;
	}

	switch(function->type())
	{
	case Function::Scene:
	{
		newFunction =
			_app->doc()->newFunction(Function::Scene, fxi_id);

		Scene* scene = static_cast<Scene*> (newFunction);

		scene->copyFrom(static_cast<Scene*> (function), fxi_id);
	}
	break;

	case Function::Chaser:
	{
		newFunction =
			_app->doc()->newFunction(Function::Chaser, KNoID);

		Chaser* chaser = static_cast<Chaser*> (newFunction);

		chaser->copyFrom(static_cast<Chaser*> (function));
	}
	break;

	case Function::Collection:
	{
		newFunction =
			_app->doc()->newFunction(Function::Collection, KNoID);

		Collection* fc = static_cast<Collection*> (newFunction);
		fc->copyFrom(static_cast<Collection*> (function));
	}
	break;

	case Function::EFX:
	{
		newFunction = 
			_app->doc()->newFunction(Function::EFX, fxi_id);

		EFX* efx = static_cast<EFX*> (newFunction);

		efx->copyFrom(static_cast<EFX*> (function), fxi_id);
	}
	break;

	default:
		newFunction = NULL;
		break;
	}

	return newFunction;
}

void FunctionManager::addFunction(Function::Type type)
{
	QTreeWidgetItem* fixtureItem = m_fixtureTree->currentItem();
	QTreeWidgetItem* newItem = NULL;
	t_fixture_id fxi_id = KNoID;
	Function* function = NULL;

	if (fixtureItem == NULL)
		return;

	// We don't want the signal handler to add the function twice
	m_blockAddFunctionSignal = true;

	fxi_id = fixtureItem->text(KColumnID).toInt();

	//
	// Create the function
	//
	switch(type)
	{
	case Function::Scene:
	{
		function = _app->doc()->newFunction(Function::Scene, fxi_id);
		if (function == NULL)
			return;
		function->setName("New Scene");
	}
	break;

	case Function::Chaser:
	{
		function = _app->doc()->newFunction(Function::Chaser, KNoID);
		if (function == NULL)
			return;
		function->setName("New Chaser");
	}
	break;

	case Function::Collection:
	{
		function = _app->doc()->newFunction(Function::Collection, KNoID);
		if (function == NULL)
			return;
		function->setName("New Collection");
	}
	break;

	case Function::EFX:
	{
		function = _app->doc()->newFunction(Function::EFX, fxi_id);
		if (function == NULL)
			return;
		function->setName("New EFX");
	}
	break;

	default:
		break;
	}

	// We don't want the signal handler to add the function twice
	m_blockAddFunctionSignal = false;

	// Create a new item for the function
	newItem = new QTreeWidgetItem(m_functionTree);

	// Update the item's contents based on the function itself
	updateFunctionItem(newItem, function);

	// Clear current selection so that slotEdit() behaves correctly
	m_functionTree->clearSelection();

	// Select only the new item
	newItem->setSelected(true);

	if (slotEdit() == QDialog::Rejected)
	{
		// Edit dialog was rejected -> delete function
		deleteSelectedFunctions();
	}
	else
	{
		// Sort the list so that the new item takes its correct place
		m_functionTree->sortItems(KColumnName, Qt::AscendingOrder);

		// Make sure that the new item is visible
		m_functionTree->scrollToItem(newItem);
	}
}

QTreeWidgetItem* FunctionManager::getItem(t_function_id id,
					  QTreeWidget* listView)
{
	Q_ASSERT(listView != NULL);

	QTreeWidgetItemIterator it(listView);

	while (*it != NULL)
	{
		if ((*it)->text(KColumnID).toInt() == id)
			return *it;
		++it;
	}

	return NULL;
}
