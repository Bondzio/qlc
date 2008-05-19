/*
  Q Light Controller
  collectioneditor.cpp

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

#include "functionselection.h"
#include "collectioneditor.h"
#include "collection.h"
#include "function.h"
#include "fixture.h"
#include "app.h"
#include "doc.h"

extern App* _app;

#define KColumnFixture  0
#define KColumnFunction 1
#define KColumnType     2
#define KColumnID       3

CollectionEditor::CollectionEditor(QWidget* parent, Collection* fc)
	: QDialog(parent)
{
	Q_ASSERT(fc != NULL);
	m_original = fc;

	setupUi(this);

	m_add->setIcon(QIcon(PIXMAPS "/edit_add.png"));
	m_remove->setIcon(QIcon(PIXMAPS "/edit_remove.png"));

	connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
		this, SLOT(slotNameEdited(const QString&)));
	connect(m_add, SIGNAL(clicked()), this, SLOT(slotAdd()));
	connect(m_remove, SIGNAL(clicked()), this, SLOT(slotRemove()));

	m_fc = new Collection(this);
	m_fc->copyFrom(fc);
	Q_ASSERT(m_fc != NULL);

	m_nameEdit->setText(m_fc->name());
	setWindowTitle("Chaser editor - " + m_fc->name());

	updateFunctionList();
}

CollectionEditor::~CollectionEditor()
{
	Q_ASSERT(m_fc != NULL);
	delete m_fc;
	m_fc = NULL;
}

void CollectionEditor::slotNameEdited(const QString& text)
{
	setWindowTitle("Chaser editor - " + text);
}

void CollectionEditor::slotAdd()
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

void CollectionEditor::slotRemove()
{
	QTreeWidgetItem* item = m_tree->currentItem();
	if (item != NULL)
	{
		t_function_id id = item->text(KColumnID).toInt();
		m_fc->removeItem(id);
		delete item;
	}
}

void CollectionEditor::accept()
{
	m_fc->setName(m_nameEdit->text());
	m_original->copyFrom(m_fc);
	_app->doc()->setModified();

	QDialog::accept();
}

void CollectionEditor::updateFunctionList()
{
	m_tree->clear();

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
		QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
		item->setText(KColumnFixture, fxi_name);
		item->setText(KColumnFunction, func_name);
		item->setText(KColumnType, func_type);
		item->setText(KColumnID, s.setNum(fid));
	}
}

bool CollectionEditor::isAlreadyMember(t_function_id id)
{
	QListIterator <t_function_id> it(*m_fc->steps());

	while (it.hasNext() == true)
	{
		if (it.next() == id)
			return true;
	}

	return false;
}
