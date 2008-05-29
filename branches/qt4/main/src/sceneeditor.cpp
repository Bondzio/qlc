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
#include <QTabWidget>
#include <iostream>
#include <QLayout>
#include <QLabel>
#include <QMenu>

#include "common/qlcfixturedef.h"

#include "fixtureselection.h"
#include "fixtureconsole.h"
#include "sceneeditor.h"
#include "fixture.h"
#include "scene.h"
#include "app.h"
#include "doc.h"

using namespace std;

extern App* _app;

#define KColumnName         0
#define KColumnManufacturer 1
#define KColumnModel        2
#define KColumnID           3

SceneEditor::SceneEditor(QWidget* parent, Scene* scene) : QDialog(parent)
{
	Q_ASSERT(scene != NULL);
	m_scene = scene;

	setupUi(this);
	init();
}

SceneEditor::~SceneEditor()
{
}

void SceneEditor::init()
{
	m_addFixtureButton->setIcon(QIcon(PIXMAPS "/edit_add.png"));
	connect(m_addFixtureButton, SIGNAL(clicked()),
		this, SLOT(slotAddFixtureClicked()));

	m_removeFixtureButton->setIcon(QIcon(PIXMAPS "/edit_remove.png"));
	connect(m_removeFixtureButton, SIGNAL(clicked()),
		this, SLOT(slotRemoveFixtureClicked()));

	m_nameEdit->setText(m_scene->name());

	QListIterator <SceneValue> it(*m_scene->values());
	while (it.hasNext() == true)
	{
		QTreeWidgetItem* item;
		SceneValue scv(it.next());
		
		if (fixtureItem(scv.fxi) == NULL)
		{
			Fixture* fixture = _app->doc()->fixture(scv.fxi);
			Q_ASSERT(fixture != NULL);

			addFixtureItem(fixture);
			addFixtureTab(fixture);
		}
	}
}

void SceneEditor::accept()
{
	m_scene->setName(m_nameEdit->text());
	QDialog::accept();
}

/*****************************************************************************
 * General page
 *****************************************************************************/

QTreeWidgetItem* SceneEditor::fixtureItem(t_fixture_id fxi_id)
{
	QList <QTreeWidgetItem*> list(
		m_tree->findItems(QString("%d").arg(fxi_id),
					 Qt::MatchExactly, KColumnID));
	if (list.count() > 0)
		return list.first();
	else
		return NULL;
}

void SceneEditor::addFixtureItem(Fixture* fixture)
{
	QTreeWidgetItem* item;

	Q_ASSERT(fixture != NULL);

	item = new QTreeWidgetItem(m_tree);
	item->setText(KColumnName, fixture->name());
	item->setText(KColumnID, QString("%1").arg(fixture->id()));
	
	if (fixture->fixtureDef() == NULL)
	{
		item->setText(KColumnManufacturer, tr("Generic"));
		item->setText(KColumnModel, tr("Generic"));
	}
	else
	{
		item->setText(KColumnManufacturer,
			      fixture->fixtureDef()->manufacturer());
		item->setText(KColumnModel, fixture->fixtureDef()->model());
	}
}

void SceneEditor::removeFixtureItem(Fixture* fixture)
{
	QTreeWidgetItem* item;

	Q_ASSERT(fixture != NULL);

	item = fixtureItem(fixture->id());
	delete item;
}

void SceneEditor::slotAddFixtureClicked()
{
	/* Put all fixtures already present into a list of fixtures that
	   will be disabled in the fixture selection dialog */
	QList <t_fixture_id> disabled;
	QTreeWidgetItemIterator twit(m_tree);
	while (*twit != NULL)
	{
		disabled.append((*twit)->text(KColumnID).toInt());
		twit++;
	}

	/* Get a list of new fixtures to add to the scene */
	FixtureSelection fs(this, _app->doc(), true, disabled);
	if (fs.exec() == QDialog::Accepted)
	{
		QTreeWidgetItem* item;
		Fixture* fixture;
		
		QListIterator <t_fixture_id> it(fs.selection);
		while (it.hasNext() == true)
		{
			fixture = _app->doc()->fixture(it.next());
			Q_ASSERT(fixture != NULL);

			addFixtureItem(fixture);
			addFixtureTab(fixture);
		}
	}
}

void SceneEditor::slotRemoveFixtureClicked()
{
	int r = QMessageBox::question(
		this, tr("Remove fixtures"),
		tr("Do you want to remove the selected fixture(s)?"),
		QMessageBox::Yes, QMessageBox::No);

	if (r == QMessageBox::Yes)
	{
		QTreeWidgetItem* item;
		t_fixture_id fxi_id;
		Fixture* fixture;

		QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());

		while (it.hasNext() == true)
		{
			item = it.next();
			fxi_id = item->text(KColumnID).toInt();
			fixture = _app->doc()->fixture(fxi_id);
			Q_ASSERT(fixture != NULL);

			removeFixtureTab(fixture);
			delete item;
		}
	}
}

void SceneEditor::addFixtureTab(Fixture* fixture)
{
	FixtureConsole* console;

	Q_ASSERT(fixture != NULL);

	console = new FixtureConsole(this, fixture->id());
	m_tab->addTab(console, fixture->name());
}

void SceneEditor::removeFixtureTab(Fixture* fixture)
{
	Q_ASSERT(fixture != NULL);

	/* Start from the first fixture tab (0 = General, 1 = All) */
	for (int i = 2; i < m_tab->count(); i++)
	{
		if (m_tab->tabText(i) == fixture->name())
		{
			m_tab->removeTab(i);
			break;
		}
	}
}
