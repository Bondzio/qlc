/*
  Q Light Controller
  functionselection.cpp

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
#include <QTreeWidget>

#include "functionselection.h"
#include "function.h"
#include "fixture.h"
#include "doc.h"

#define KColumnName 0
#define KColumnType 1
#define KColumnID   2

FunctionSelection::FunctionSelection(QWidget* parent, Doc* doc, bool multiple,
				     t_function_id disableFunction)
	: QDialog(parent)
{
	Q_ASSERT(doc != NULL);

	setupUi(this);

	/* Multiple/single selection */
	if (multiple == true)
		m_tree->setSelectionMode(QAbstractItemView::MultiSelection);
	else
		m_tree->setSelectionMode(QAbstractItemView::SingleSelection);

	/* Fill the tree */
	for (t_function_id fid = 0; fid < KFunctionArraySize; fid++)
	{
		Function* function;
		QString str;
		
		function = doc->function(fid);
		if (function == NULL)
			continue;

		QTreeWidgetItem* item;
		QTreeWidgetItem* parent;
		
		parent = fixtureItem(function->fixture(), doc);
		item = new QTreeWidgetItem(parent);

		item->setText(KColumnName, function->name());
		item->setText(KColumnType, function->typeString());
		item->setText(KColumnID, str.setNum(fid));

		if (disableFunction == fid)
			item->setFlags(0); // Disables the item
	}
}

FunctionSelection::~FunctionSelection()
{
}

QTreeWidgetItem* FunctionSelection::fixtureItem(t_fixture_id fxi_id, Doc* doc)
{
	QTreeWidgetItem* parent;
	Fixture* fxi;
	QString str;

	Q_ASSERT(doc != NULL);

	/* Search for an existing fixture item to use as a parent */
	for (int i = 0; i < m_tree->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item = m_tree->topLevelItem(i);
		if (item->text(KColumnID).toInt() == fxi_id)
			return item; // Fixture item found, return immediately
	}

	/* Fixture item was not found, so create one. */
	fxi = doc->fixture(fxi_id);
	if (fxi != NULL)
	{
		/* The function is a fixture function (Scene/EFX) */
		parent = new QTreeWidgetItem(m_tree);
		parent->setText(KColumnName, fxi->name());
		parent->setText(KColumnID, str.setNum(fxi->id()));
	}
	else
	{
		/* The function is a global function (Chaser/Collection) */
		parent = new QTreeWidgetItem(m_tree);
		parent->setText(KColumnName, tr("Global"));
		parent->setText(KColumnID, str.setNum(KNoID));
	}

	/* Removes all other flags (including Qt::ItemIsSelectable)
	   which should prevent us from having to check, whether the user
	   has selected fixtures instead of functions. */
	parent->setFlags(Qt::ItemIsEnabled);

	return parent;
}

void FunctionSelection::accept()
{
	selection.clear();

	/* TODO: Check, whether some items are fixture items. If they are,
	   don't put them into selection list. See above Qt::ItemIsEnabled. */
	QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
	while (it.hasNext() == true)
		selection.append(it.next()->text(KColumnID).toInt());

	QDialog::accept();
}
