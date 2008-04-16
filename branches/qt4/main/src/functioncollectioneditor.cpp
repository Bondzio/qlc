/*
  Q Light Controller
  functioncollectioneditor.cpp

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
#include <QMessageBox>
#include <QLineEdit>
#include <QLabel>
#include <QTimer>
#include <QIcon>

#include "common/qlcfixturedef.h"

#include "functioncollectioneditor.h"
#include "functioncollection.h"
#include "functionselection.h"
#include "function.h"
#include "fixture.h"
#include "app.h"
#include "doc.h"

extern App* _app;

FunctionCollectionEditor::FunctionCollectionEditor(QWidget* parent,
						   FunctionCollection* fc)
	: QDialog(parent)
{
	Q_ASSERT(fc != NULL);
	m_original = fc;

	m_fc = new FunctionCollection();
	m_fc->copyFrom(fc);
	Q_ASSERT(m_fc != NULL);

	setupUi(this);

	m_addButton->setIcon(QIcon(PIXMAPS "/edit_add.png"));
	connect(m_addButton, SIGNAL(clicked()), this, SLOT(slotAdd()));

	m_removeButton->setIcon(QIcon(PIXMAPS "/edit_remove.png"));
	connect(m_removeButton, SIGNAL(clicked()), this, SLOT(slotRemove()));

	m_nameEdit->setText(m_fc->name());
	updateFunctionList();
}

FunctionCollectionEditor::~FunctionCollectionEditor()
{
	Q_ASSERT(m_fc != NULL);
	delete m_fc;
	m_fc = NULL;
}

void FunctionCollectionEditor::slotAdd()
{
	FunctionSelection sel(this, _app->doc(), true, m_fc->id());
	if (sel.exec() == QDialog::Accepted)
	{
		t_function_id fid;

		QListIterator <t_function_id> it(sel.selection);
		while (it.hasNext() == true)
		{
			fid = it.next();
			if (isAlreadyMember(fid) == false)
				m_fc->addItem(fid);
		}

		updateFunctionList();
	}
}

void FunctionCollectionEditor::slotRemove()
{
	QTreeWidgetItem* item = m_functionList->currentItem();
	if (item != NULL)
	{
		t_function_id id = item->text(2).toInt();
		if (m_fc->removeItem(id) == true)
			delete item;
		else
			Q_ASSERT(false);
	}
}

void FunctionCollectionEditor::accept()
{
	m_fc->setName(m_nameEdit->text());
	m_original->copyFrom(m_fc);
	_app->doc()->setModified();

	QDialog::accept();
}

void FunctionCollectionEditor::updateFunctionList()
{
	m_functionList->clear();

	QListIterator <t_function_id> it(*m_fc->steps());
	while (it.hasNext() == true)
	{
		QString fxi_name;
		QString func_name;
		QString func_type;

		t_function_id fid = it.next();
		Function* function = _app->doc()->function(fid);

		if (function == NULL)
		{
			func_name = "Invalid";
			fxi_name = "Invalid";
			func_type = "Invalid";
		}
		else if (function->fixture() != KNoID)
		{
			Fixture* fxi = NULL;

			func_name = function->name();
			func_type = Function::typeToString(function->type());

			fxi = _app->doc()->fixture(function->fixture());
			if (fxi == NULL)
				fxi_name = "Invalid";
			else
				fxi_name = fxi->name();
		}
		else
		{
			fxi_name = QString("Global");
			func_name = function->name();
			func_type = Function::typeToString(function->type());
		}

		QString s;
		QTreeWidgetItem* item = new QTreeWidgetItem(m_functionList);
		item->setText(0, fxi_name);
		item->setText(1, func_name);
		item->setText(2, func_type);
		item->setText(3, s.setNum(fid));
	}
}

bool FunctionCollectionEditor::isAlreadyMember(t_function_id id)
{
	QListIterator <t_function_id> it(*m_fc->steps());

	while (it.hasNext() == true)
	{
		if (it.next() == id)
			return true;
	}

	return false;
}
