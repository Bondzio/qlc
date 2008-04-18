/*
  Q Light Controller
  pluginmanager.cpp

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

#include <QTreeWidgetItem>
#include <QTextBrowser>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QStringList>
#include <QSplitter>
#include <QToolBar>
#include <QWidget>
#include <QString>
#include <QMenu>
#include <QIcon>

#include "pluginmanager.h"
#include "inputmap.h"
#include "dmxmap.h"
#include "app.h"

extern App* _app;

static const QString KInputNode ("Input");
static const QString KOutputNode ("Output");

#define KColumnName 0

/*****************************************************************************
 * Initialization
 *****************************************************************************/

PluginManager::PluginManager(QWidget* parent) : QWidget(parent)
{
	new QVBoxLayout(this);

	setWindowTitle("Plugin Manager");
	setWindowIcon(QIcon(PIXMAPS "/plugin.png"));

	initActions();
	initToolBar();
	initDataView();

	fillPlugins();
	slotSelectionChanged();
}

PluginManager::~PluginManager()
{
}

void PluginManager::initActions()
{
	m_configureAction = new QAction(QIcon(PIXMAPS "/configure.png"),
					"Configure plugin", this);
	connect(m_configureAction, SIGNAL(triggered(bool)),
		this, SLOT(slotConfigure()));

	m_outputMapAction = new QAction(QIcon(PIXMAPS "/attach.png"),
					"Output Map", this);
	connect(m_outputMapAction, SIGNAL(triggered(bool)),
		this, SLOT(slotOutputMap()));

	m_inputMapAction = new QAction(QIcon(PIXMAPS "/attach.png"),
				       "Input Map", this);
	connect(m_inputMapAction, SIGNAL(triggered(bool)),
		this, SLOT(slotInputMap()));
}

void PluginManager::initToolBar()
{
	// Add a toolbar to the dock area
	m_toolbar = new QToolBar("Plugin Manager", this);
	layout()->addWidget(m_toolbar);
	m_toolbar->addAction(m_configureAction);
	m_toolbar->addAction(m_outputMapAction);
	m_toolbar->addAction(m_inputMapAction);
}

void PluginManager::initDataView()
{
	// Create a splitter to divide list view and text view
	m_splitter = new QSplitter(this);
	m_splitter->setSizePolicy(QSizePolicy::Expanding,
				  QSizePolicy::Expanding);
	layout()->addWidget(m_splitter);

	// Create the list view
	m_listView = new QTreeWidget(this);
	m_splitter->addWidget(m_listView);
	m_listView->setHeaderLabels(QStringList("Plugins"));
/*
	m_splitter->setResizeMode(m_listView, QSplitter::Auto);

	m_listView->setRootIsDecorated(true);
	m_listView->setMultiSelection(false);
	m_listView->setAllColumnsShowFocus(true);
	m_listView->setSorting(KColumnName, true);
	m_listView->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

	m_listView->header()->setClickEnabled(true);
	m_listView->header()->setResizeEnabled(false);
	m_listView->header()->setMovingEnabled(false);

	m_listView->addColumn("Plugins");
	m_listView->setResizeMode(QListView::LastColumn);
*/
	connect(m_listView, SIGNAL(selectionChanged()),
		this, SLOT(slotSelectionChanged()));
  
	connect(m_listView, SIGNAL(doubleClicked(QTreeWidgetItem*)),
		this, SLOT(slotConfigure()));

	connect(m_listView, SIGNAL(customContextMenuRequested(const QPoint&)),
		this, SLOT(slotContextMenuRequested(const QPoint&)));

	// Create the text view
	m_textView = new QTextBrowser(this);
	m_splitter->addWidget(m_textView);
	//m_splitter->setResizeMode(m_textView, QSplitter::Auto);
}

void PluginManager::fillPlugins()
{
	QTreeWidgetItem* parent;
	QTreeWidgetItem* item;
	QStringList::iterator it;
	QStringList list;

	m_listView->clear();

	/* Output plugins */
	parent = new QTreeWidgetItem(m_listView);
	parent->setText(0, KOutputNode);
	parent->setExpanded(true);
	list = _app->dmxMap()->pluginNames();
	for (it = list.begin(); it != list.end(); ++it)
	{
		item = new QTreeWidgetItem(parent);
		item->setText(KColumnName, *it);
	}

	/* Input plugins */
	parent = new QTreeWidgetItem(m_listView);
	parent->setText(0, KInputNode);
	parent->setExpanded(true);
	list = _app->inputMap()->pluginNames();
	for (it = list.begin(); it != list.end(); ++it)
	{
		item = new QTreeWidgetItem(parent);
		item->setText(KColumnName, *it);
	}
}

/*****************************************************************************
 * Menu & tool button slots
 *****************************************************************************/

void PluginManager::slotConfigure()
{
	QTreeWidgetItem* item;
	QString parentName;
	
	item = m_listView->currentItem();
	if (item != NULL && item->parent() != NULL)
	{
		/* Find out the parent node name to determine, whether
		   the plugin belongs to the input map or output map */
		parentName = item->parent()->text(KColumnName);

		if (parentName == KOutputNode)
		{
			_app->dmxMap()->configurePlugin(
				item->text(KColumnName));
		}
		else if (parentName == KInputNode)
		{
			_app->inputMap()->configurePlugin(
				item->text(KColumnName));
		}

		slotSelectionChanged();
	}
}


void PluginManager::slotOutputMap()
{
	_app->dmxMap()->openEditor(this);
	slotSelectionChanged();
}

void PluginManager::slotInputMap()
{
	QMessageBox::information(this, "TODO", "TODO");
}

/*****************************************************************************
 * List view slots
 *****************************************************************************/

void PluginManager::slotSelectionChanged()
{
	QTreeWidgetItem* item = m_listView->currentItem();
	QString status;
	QString name;
	QString parent;

	if (item != NULL)
	{
		name = item->text(KColumnName);

		if (item->parent() == NULL)
		{
			m_configureAction->setEnabled(false);
			if (name == KOutputNode)
				status = _app->dmxMap()->pluginStatus();
			else if (name == KInputNode)
				status = _app->inputMap()->pluginStatus();
		}
		else
		{
			m_configureAction->setEnabled(true);
			parent = item->parent()->text(KColumnName);

			if (parent == KOutputNode)
				status = _app->dmxMap()->pluginStatus(name);
			else if (parent == KInputNode)
				status = _app->inputMap()->pluginStatus(name);
		}
	}
	else
	{
		m_configureAction->setEnabled(false);
	}

	if (status.length() == 0)
	{
		status = QString("<FONT SIZE=\"5\">No selection</FONT>\n");
		status += QString("<BR>\n");
		status += QString("Select a plugin to begin.");
	}

	m_textView->setText(status);
}

void PluginManager::slotContextMenuRequested(const QPoint& point)
{
	QTreeWidgetItem* item = m_listView->currentItem();
	if (item == NULL)
		return;

	QMenu menu(this);
	if (item != NULL)
		menu.addAction(m_configureAction);
	menu.addAction(m_outputMapAction);
	menu.addAction(m_inputMapAction);
	menu.exec(point);
}

/*****************************************************************************
 * Default settings
 *****************************************************************************/

void PluginManager::saveDefaults(const QString& path)
{
	/* TODO */
}

void PluginManager::loadDefaults(const QString& path)
{
	/* TODO */
}
