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

/*****************************************************************************
 * Initialization
 *****************************************************************************/

FunctionSelection::FunctionSelection(QWidget* parent, Doc* doc, bool multiple,
				     t_function_id disableFunction,
				     Function::Type filter)
	: QDialog(parent)
{
	Q_ASSERT(doc != NULL);

	setupUi(this);

	/* Multiple/single selection */
	if (multiple == true)
		m_tree->setSelectionMode(QAbstractItemView::MultiSelection);
	else
		m_tree->setSelectionMode(QAbstractItemView::SingleSelection);

	connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(slotItemDoubleClicked(QTreeWidgetItem*,int)));

	/* Fill the tree */
	for (t_function_id fid = 0; fid < KFunctionArraySize; fid++)
	{
		QTreeWidgetItem* item;
		Function* function;
		QString str;
		
		function = doc->function(fid);
		if (function == NULL)
			continue;

		if (filter == Function::Undefined || filter == function->type())
		{
			item = new QTreeWidgetItem(m_tree);
			item->setText(KColumnName, function->name());
			item->setText(KColumnType, function->typeString());
			item->setText(KColumnID, str.setNum(fid));

			if (disableFunction == fid)
				item->setFlags(0); // Disables the item
		}
	}
}

FunctionSelection::~FunctionSelection()
{
}

/*****************************************************************************
 * Internal
 *****************************************************************************/

void FunctionSelection::slotItemDoubleClicked(QTreeWidgetItem* item, int col)
{
	if (item == NULL)
		return;

	accept();
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
