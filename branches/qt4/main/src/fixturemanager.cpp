/*
  Q Light Controller
  fixturemanager.cpp

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
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QMessageBox>
#include <QByteArray>
#include <QSplitter>
#include <QToolBar>
#include <iostream>
#include <QAction>
#include <QWidget>
#include <QString>
#include <QIcon>
#include <QMenu>
#include <QtXml>

#include "common/qlcfixturemode.h"
#include "common/qlcfixturedef.h"
#include "common/qlccapability.h"
#include "common/qlcchannel.h"
#include "common/qlcfile.h"

#include "functioncollection.h"
#include "fixtureproperties.h"
#include "consolechannel.h"
#include "fixturemanager.h"
#include "addfixture.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "app.h"
#include "doc.h"
#include "efx.h"

extern App* _app;

using namespace std;

// List view column numbers
#define KColumnUniverse 0
#define KColumnAddress  1
#define KColumnName     2
#define KColumnID       3

// Default window size
#define KDefaultWidth  600
#define KDefaultHeight 300

/*****************************************************************************
 * Initialization
 *****************************************************************************/

FixtureManager::FixtureManager(QWidget* parent) : QWidget(parent)
{
	new QVBoxLayout(this);

	// Name and icon
	setWindowTitle(QString("Fixture Manager"));
	setWindowIcon(QIcon(PIXMAPS "/fixture.png"));

	initActions();
	initToolBar();
	initDataView();
	updateView();
}

FixtureManager::~FixtureManager()
{
}

/*****************************************************************************
 * Doc signal handlers
 *****************************************************************************/

void FixtureManager::slotFixtureAdded(t_fixture_id id)
{
	Fixture* fxi;

	fxi = _app->doc()->fixture(id);
	if (fxi != NULL)
	{
		// Create a new list view item
		QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);

		// Fill fixture information to the item
		updateItem(item, fxi);

		/* Select the item (fixtures can be added only manually,
		   so if this signal comes, a fixture was added with
		   the fixture manager) */
		m_tree->setCurrentItem(item);
	}
}

void FixtureManager::slotFixtureRemoved(t_fixture_id id)
{
	QTreeWidgetItemIterator it(m_tree);
	while (*it != NULL)
	{
		if ((*it)->text(KColumnID).toInt() == id)
		{
			if ((*it)->isSelected() == true)
			{
				QTreeWidgetItem* nextItem;

				// Try to select the closest neighbour
				if (m_tree->itemAbove(*it) != NULL)
					nextItem = m_tree->itemAbove(*it);
				else
					nextItem = m_tree->itemBelow(*it);
				m_tree->setCurrentItem(nextItem);
			}

			delete (*it);
			break;
		}

		++it;
	}
}

void FixtureManager::slotModeChanged()
{
	if (_app->mode() == App::Operate)
	{
		m_addAction->setEnabled(false);
		m_propertiesAction->setEnabled(false);
		m_cloneAction->setEnabled(false);
		m_removeAction->setEnabled(false);
	}
	else
	{
		m_addAction->setEnabled(true);
		m_propertiesAction->setEnabled(true);
		m_cloneAction->setEnabled(true);
		m_removeAction->setEnabled(true);
	}

	slotSelectionChanged();
}

/*****************************************************************************
 * Data view
 *****************************************************************************/

void FixtureManager::initDataView()
{
	// Create a splitter to divide list view and text view
	m_splitter = new QSplitter(Qt::Horizontal, this);
	layout()->addWidget(m_splitter);
	m_splitter->setSizePolicy(QSizePolicy::Expanding,
				  QSizePolicy::Expanding);

	// Create the list view
	m_tree = new QTreeWidget(this);
	m_splitter->addWidget(m_tree);

	QStringList labels;
	labels << "Universe" << "Address" << "Name";
	m_tree->setHeaderLabels(labels);
	m_tree->setRootIsDecorated(false);
	m_tree->setSortingEnabled(true);

	connect(m_tree, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotSelectionChanged()));

	connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(slotDoubleClicked(QTreeWidgetItem*)));

	connect(m_tree, SIGNAL(customContextMenuRequested(const QPoint&)),
		this, SLOT(slotContextMenuRequested(const QPoint&)));

	// Create the text view
	m_info = new QTextBrowser(this);
	m_splitter->addWidget(m_info);

	m_splitter->setStretchFactor(0, 1);
	m_splitter->setStretchFactor(1, 0);

	slotSelectionChanged();
}

void FixtureManager::updateView()
{
	QTreeWidgetItem* item;
	t_fixture_id currentId = KNoID;
	t_fixture_id id = KNoID;
	Fixture* fxt = NULL;

	// Store the currently selected fixture's ID
	item = m_tree->currentItem();
	if (item != NULL)
		currentId = item->text(KColumnID).toInt();

	// Clear the view
	m_tree->clear();

	// Add all fixtures
	for (id = 0; id < KFixtureArraySize; id++)
	{
		fxt = _app->doc()->fixture(id);
		if (fxt == NULL)
			continue;

		item = new QTreeWidgetItem(m_tree);

		// Update fixture information to the item
		updateItem(item, fxt);

		// Select this if it was selected before update
		if (currentId == id)
			item->setSelected(true);
	}

	slotSelectionChanged();
}

void FixtureManager::updateItem(QTreeWidgetItem* item, Fixture* fxi)
{
	Q_ASSERT(item != NULL);
	Q_ASSERT(fxi != NULL);

	// Universe column
	item->setText(KColumnUniverse,
		      QString("%1").arg(fxi->universe() + 1));

	// Address column
	QString s;
	s.sprintf("%.3d - %.3d", fxi->address() + 1,
		  fxi->address() + fxi->channels());
	item->setText(KColumnAddress, s);

	// Name column
	item->setText(KColumnName, fxi->name());

	// ID column
	item->setText(KColumnID, QString("%1").arg(fxi->id()));
}

void FixtureManager::copyFunction(Function* function, Fixture* fxi)
{
	switch(function->type())
	{
	case Function::Scene:
	{
		Scene* scene = static_cast<Scene*>
			(_app->doc()->newFunction(Function::Scene, 
						  fxi->id()));

		scene->copyFrom(static_cast<Scene*> (function), fxi->id());
	}
	break;

	case Function::Chaser:
	{
		Chaser* chaser = static_cast<Chaser*>
			(_app->doc()->newFunction(Function::Chaser, KNoID));

		chaser->copyFrom(static_cast<Chaser*> (function));
	}
	break;

	case Function::Collection:
	{
		FunctionCollection* fc = static_cast<FunctionCollection*>
			(_app->doc()->newFunction(Function::Collection, KNoID));

		fc->copyFrom(static_cast<FunctionCollection*> (function));
	}
	break;

	case Function::EFX:
	{
		EFX* efx = static_cast<EFX*>
			(_app->doc()->newFunction(Function::EFX, KNoID));

		efx->copyFrom(static_cast<EFX*> (function), fxi->id());
	}
	break;

	default:
		break;
	}
}

void FixtureManager::slotSelectionChanged()
{
	QTreeWidgetItem* item = m_tree->currentItem();
	if (item == NULL)
	{
		// Add is not available in operate mode
		if (_app->mode() == App::Design)
			m_addAction->setEnabled(true);
		else
			m_addAction->setEnabled(false);

		// Disable all other actions
		m_removeAction->setEnabled(false);
		m_consoleAction->setEnabled(false);
		m_cloneAction->setEnabled(false);
		m_propertiesAction->setEnabled(false);

		QString info;
		info = QString("<HTML><BODY>");
		info += QString("<H1>No fixtures</H1>");
		info += QString("Click \"Add\" on the ");
		info += QString("toolbar to add a new fixture.");
		info += QString("</BODY></HTML>");
		m_info->setText(info);
	}
	else
	{
		Fixture* fxi;
		t_fixture_id id;

		// Set the text view's contents
		id = item->text(KColumnID).toInt();
		fxi = _app->doc()->fixture(id);
		Q_ASSERT(fxi != NULL);

		m_info->setText(fxi->status());

		// Enable/disable actions
		if (_app->mode() == App::Design)
		{
			m_addAction->setEnabled(true);
			m_removeAction->setEnabled(true);
			m_consoleAction->setEnabled(true);
			m_cloneAction->setEnabled(true);
			m_propertiesAction->setEnabled(true);
		}
		else
		{
			m_addAction->setEnabled(false);
			m_removeAction->setEnabled(false);
			m_consoleAction->setEnabled(false);
			m_cloneAction->setEnabled(false);
			m_propertiesAction->setEnabled(false);
		}
	}
}

void FixtureManager::slotDoubleClicked(QTreeWidgetItem* item)
{
	if (item != NULL && _app->mode() != App::Operate)
		slotProperties();
}

/*****************************************************************************
 * Menu, toolbar and actions
 *****************************************************************************/

void FixtureManager::initActions()
{
	m_addAction = new QAction(QIcon(PIXMAPS "/wizard.png"),
				  tr("Add fixture..."), this);
	connect(m_addAction, SIGNAL(triggered(bool)),
		this, SLOT(slotAdd()));

	m_propertiesAction = new QAction(QIcon(PIXMAPS "/configure.png"),
					 tr("Configure fixture..."), this);
	connect(m_propertiesAction, SIGNAL(triggered(bool)),
		this, SLOT(slotProperties()));

	m_cloneAction = new QAction(QIcon(PIXMAPS "/editcopy.png"),
				    tr("Clone fixture"), this);
	connect(m_cloneAction, SIGNAL(triggered(bool)),
		this, SLOT(slotClone()));

	m_consoleAction = new QAction(QIcon(PIXMAPS "/console.png"),
				      tr("Fixture console"), this);
	connect(m_consoleAction, SIGNAL(triggered(bool)),
		this, SLOT(slotConsole()));

	m_removeAction = new QAction(QIcon(PIXMAPS "/editdelete.png"),
				     tr("Remove fixture"), this);
	connect(m_removeAction, SIGNAL(triggered(bool)),
		this, SLOT(slotRemove()));
}

void FixtureManager::initToolBar()
{
	m_toolbar = new QToolBar("Fixture Manager", this);
	layout()->addWidget(m_toolbar);
	m_toolbar->addAction(m_addAction);
	m_toolbar->addSeparator();
	m_toolbar->addAction(m_propertiesAction);
	m_toolbar->addAction(m_cloneAction);
	m_toolbar->addAction(m_consoleAction);
	m_toolbar->addSeparator();
	m_toolbar->addAction(m_removeAction);
}

void FixtureManager::slotAdd()
{
	AddFixture af(this);
	if (af.exec() == QDialog::Accepted)
	{
		QString name = af.name();
		t_channel address = af.address();
		t_channel universe = af.universe();
		t_channel channels = af.channels();
		int gap = af.gap();

		QLCFixtureDef* fixtureDef = af.fixtureDef();
		QLCFixtureMode* mode = af.mode();

		if (fixtureDef != NULL && mode != NULL)
		{
			/* Add a normal fixture with an existing definition */

			/* If an empty name was given use the model instead */
			if (name.simplified() == QString::null)
				name = fixtureDef->model();

			// Add the first fixture without gap
			_app->doc()->newFixture(fixtureDef, mode, address,
						universe, name);

			// Add the rest (if any) with address gap
			for (int i = 1; i < af.amount(); i++)
			{
				_app->doc()->newFixture(fixtureDef, mode,
					address + (i * channels) + gap,
					universe, name);
			}
		}
		else
		{
			/* Add a generic fixture without definition */

			/* If an empty name was given use Generic instead */
			if (name.simplified() == QString::null)
				name = KXMLFixtureGeneric;

			// Add the first fixture without gap
			_app->doc()->newGenericFixture(address, universe,
						       channels, name);

			// Add the rest (if any) with address gap
			for (int i = 1; i < af.amount(); i++)
			{
				_app->doc()->newGenericFixture(
					address + (i * channels) + gap,
					universe, channels, name);
			}
		}
	}
}

void FixtureManager::slotProperties()
{
	QTreeWidgetItem* item = m_tree->currentItem();
	if (item != NULL)
	{
		t_fixture_id id = item->text(KColumnID).toInt();
		Fixture* fixture = _app->doc()->fixture(id);
		Q_ASSERT(fixture != NULL);

		// View properties dialog
		FixtureProperties prop(this, id);
		if (prop.exec() == QDialog::Accepted)
		{
			// Update changes to view
			updateItem(item, fixture);

			// Update changes to the info view
			slotSelectionChanged();
		}
	}
}

void FixtureManager::slotConsole()
{
	QTreeWidgetItem* item = m_tree->currentItem();
	if (item != NULL)
	{
		t_fixture_id id = item->text(KColumnID).toInt();
		Fixture* fxi = _app->doc()->fixture(id);
		Q_ASSERT(fxi != NULL);

		fxi->viewConsole();
	}
}

void FixtureManager::slotClone()
{
	QLCFixtureMode* fixtureMode;
	QLCFixtureDef* fixtureDef;
	QTreeWidgetItem* item;
	t_fixture_id old_id;
	Fixture* old_fxi;
	Fixture* new_fxi;
	QString new_name;

	/* Get the selected listview item */
	item = m_tree->currentItem();
	Q_ASSERT(item != NULL);

	/* Get the old fixture instance */
	old_id = item->text(KColumnID).toInt();
	old_fxi = _app->doc()->fixture(old_id);
	Q_ASSERT(old_fxi != NULL);

	/* Get the old fixture instance's fixture definition */
	fixtureDef = old_fxi->fixtureDef();
	Q_ASSERT(fixtureDef != NULL);

	/* Get the old fixture instance's mode */
	fixtureMode = old_fxi->fixtureMode();
	Q_ASSERT(fixtureMode != NULL);

	new_name = "Copy of ";
	new_name += item->text(KColumnName);

	// Add new fixture
	new_fxi = _app->doc()->newFixture(fixtureDef, fixtureMode,
					  0, 0, new_name);
	if (new_fxi != NULL)
	{
		for (t_function_id id = 0; id < KFunctionArraySize; id++)
		{
			Function* function = _app->doc()->function(id);
			if (function == NULL)
				continue;

			// Copy only functions that belong to the old fixture
			if (function->fixture() == old_id)
				copyFunction(function, new_fxi);
		}

		/* Open properties so that the user can rename the fixture,
		   set its address, etc... */
		slotProperties();
	}
}

void FixtureManager::slotRemove()
{
	QTreeWidgetItem* item = m_tree->currentItem();
	if (item == NULL)
		return;

	// Get the fixture id
	t_fixture_id id = item->text(KColumnID).toInt();

	// Display a question
	if (QMessageBox::question(this, "Remove fixture",
				  QString("Do you want to remove %1?")
					.arg(item->text(KColumnName)),
				  QMessageBox::Yes, QMessageBox::No)
	    == QMessageBox::Yes)
	{
		_app->doc()->deleteFixture(id);
	}
}

void FixtureManager::slotAutoFunction()
{
	QTreeWidgetItem* item;
	t_fixture_id fxi_id;
	Fixture* fxi;

	item = m_tree->currentItem();
	if (item == NULL)
		return;

	fxi_id = item->text(KColumnID).toInt();
	fxi = _app->doc()->fixture(fxi_id);
	Q_ASSERT(fxi != NULL);

	// Loop over all channels
	for (int i = 0; i < fxi->channels(); i++)
	{
		QLCChannel* channel = fxi->channel(i);
		Q_ASSERT(channel != NULL);

		QListIterator <QLCCapability*> 
			cap_it(*channel->capabilities());

		// Loop over all capabilities
		while (cap_it.hasNext() == true)
		{
			QLCCapability* cap = cap_it.next();
			Q_ASSERT(cap != NULL);

			Scene* sc = static_cast<Scene*> 
				(_app->doc()->newFunction(Function::Scene,
							  fxi_id));
			sc->setName(channel->name() + " - " + cap->name());

			// Set the unused channels to NoSet and zero.
			for (int j = 0; j < fxi->channels(); j++)
				sc->set(j, 0, Scene::NoSet);

			// Set only the capability
			sc->set(i, (t_value) ((cap->min() + cap->max()) / 2),
				Scene::Set);
		}
	}

	fxi->viewConsole();
}

void FixtureManager::slotContextMenuRequested(const QPoint& point)
{
	QMenu menu(this);
	menu.addAction(m_addAction);
	menu.addSeparator();
	menu.addAction(m_propertiesAction);
	menu.addAction(m_cloneAction);
	menu.addAction(m_consoleAction);
	menu.addSeparator();
	menu.addAction(m_removeAction);
	menu.exec(point);
}

/*****************************************************************************
 * Load & save
 *****************************************************************************/

void FixtureManager::loader(QDomDocument* doc, QDomElement* root)
{
	/* TODO !!! */
	//_app->createFixtureManager();
	//_app->fixtureManager()->loadXML(doc, root);
}

bool FixtureManager::loadXML(QDomDocument* doc, QDomElement* root)
{
	bool visible = false;
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
	int splitter_left = 0;
	int splitter_right = 0;

	QDomNode node;
	QDomElement tag;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCFixtureManager)
	{
		cout << "Fixture Manager node not found!" << endl;
		return false;
	}

	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		if (tag.tagName() == KXMLQLCWindowState)
		{
			QLCFile::loadXMLWindowState(&tag, &x, &y, &w, &h,
						    &visible);
		}
		else
		{
			cout << "Unknown fixture manager tag: "
			     << tag.tagName().toStdString()
			     << endl;
		}

		node = node.nextSibling();
	}

	hide();
	setGeometry(x, y, w, h);
	if (visible == false)
		showMinimized();
	else
		showNormal();

	m_splitter->restoreState(
		root->attribute(KXMLQLCFixtureManagerSplitterSize).toAscii());

	return true;
}

bool FixtureManager::saveXML(QDomDocument* doc, QDomElement* fxi_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(fxi_root != NULL);

	/* Fixture Manager entry */
	root = doc->createElement(KXMLQLCFixtureManager);
	fxi_root->appendChild(root);

	// Splitter size
	root.setAttribute(KXMLQLCFixtureManagerSplitterSize,
			  (const char*) m_splitter->saveState());

	/* Save window state. parentWidget() should be used for all
	   widgets within the workspace. */
	return QLCFile::saveXMLWindowState(doc, &root, parentWidget());
}
