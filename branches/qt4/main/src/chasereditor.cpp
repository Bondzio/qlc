/*
  Q Light Controller
  chasereditor.cpp

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
#include <QRadioButton>
#include <QTreeWidget>
#include <QPushButton>
#include <QMessageBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QTimer>
#include <QIcon>

#include "common/qlcfixturedef.h"

#include "functionselection.h"
#include "chasereditor.h"
#include "fixture.h"
#include "chaser.h"
#include "app.h"
#include "doc.h"

extern App* _app;

#define KColumnNumber     0
#define KColumnFixture    1
#define KColumnFunction   2
#define KColumnType       3
#define KColumnFunctionID 4

ChaserEditor::ChaserEditor(QWidget* parent, Chaser* chaser)
	: QDialog(parent)
{
	Q_ASSERT(chaser != NULL);

	m_chaser = new Chaser();
	m_chaser->copyFrom(chaser);
	Q_ASSERT(m_chaser != NULL);

	m_original = chaser;
	m_functionSelection = NULL;

	/* Name edit */
	m_nameEdit->setText(m_chaser->name());
	m_nameEdit->setSelection(0, m_nameEdit->text().length());

	/* Button pixmaps */
	m_addButton->setIcon(QIcon(PIXMAPS "/edit_add.png"));
	m_removeButton->setIcon(QIcon(PIXMAPS "/edit_remove.png"));
	m_raiseButton->setIcon(QIcon(PIXMAPS "/up.png"));
	m_lowerButton->setIcon(QIcon(PIXMAPS "/down.png"));

	/* Running order */
	switch (m_chaser->runOrder())
	{
	default:
	case Chaser::Loop:
		m_loop->setChecked(true);
		break;
	case Chaser::PingPong:
		m_pingPong->setChecked(true);
		break;
	case Chaser::SingleShot:
		m_singleShot->setChecked(true);
		break;
	}

	/* Running direction */
	switch (m_chaser->direction())
	{
	default:
	case Chaser::Forward:
		m_forward->setChecked(true);
		break;
	case Chaser::Backward:
		m_backward->setChecked(true);
		break;
	}

	m_stepList->setSortingEnabled(false);
	updateStepList();
}

ChaserEditor::~ChaserEditor()
{
	delete m_chaser;
}

void ChaserEditor::updateStepList(int selectIndex)
{
	QTreeWidgetItem* item;
	QString fxi_name;
	QString func_name;
	QString func_type;
	Fixture* fxi = NULL;

	m_stepList->clear();

	QListIterator <t_function_id> it(*m_chaser->steps());
	it.toBack();
	while (it.hasPrevious() == true)
	{
		t_function_id fid;
		Function* function;
		QString str;

		fid = it.previous();
		function = _app->doc()->function(fid);
		if (function == NULL)
		{
			fxi_name = QString("Invalid");
			func_name = QString("Invalid");
			func_type = QString("Invalid");
		}
		else if (function->fixture() != KNoID)
		{
			func_name = function->name();
			func_type = Function::typeToString(function->type());
			
			fxi = _app->doc()->fixture(function->fixture());
			if (fxi == NULL)
				fxi_name = QString("Invalid");
			else
				fxi_name = fxi->name();
		}
		else
		{
			fxi_name = QString("Global");
			func_name = function->name();
			func_type = Function::typeToString(function->type());
		}
		
		item = new QTreeWidgetItem(m_stepList);
		item->setText(KColumnNumber, "###");
		item->setText(KColumnFixture, fxi_name);
		item->setText(KColumnFunction, func_name);
		item->setText(KColumnType, func_type);
		item->setText(KColumnFunctionID, str.setNum(fid));
	}

	/* Select the specified item */
	item = m_stepList->topLevelItem(selectIndex);
	if (item != NULL)
		item->setSelected(true);

	updateOrderNumbers();
}

void ChaserEditor::updateOrderNumbers()
{
	int i = 1;
	QString num;

	QTreeWidgetItemIterator it(m_stepList);
	while (*it != NULL)
	{
		num.sprintf("%.03d", i++);
		(*it)->setText(KColumnNumber, num);
	}
}

void ChaserEditor::slotAddClicked()
{
	if (m_functionSelection == NULL)
	{
		// Create a new function manager
		m_functionSelection = new FunctionSelection(this,
							    _app->doc(),
							    false,
							    m_original->id());

		while (m_functionSelection->exec() == QDialog::Accepted)
		{
			QListIterator <t_function_id>
				it(m_functionSelection->selection);
			
			while (it.hasNext() == true)
				m_chaser->addStep(it.next());
			
			// Clear the selection dialog's selection
			m_functionSelection->selection.clear();
			
			// Update all steps in the list
			updateStepList();
			
			// Add another step after a short delay
			QTimer::singleShot(250, this, SLOT(slotAddAnother()));
		}

		// Clear the last selection
		m_functionSelection->selection.clear();			
	}
}

void ChaserEditor::slotRemoveClicked()
{
	QTreeWidgetItem* item = m_stepList->currentItem();

	if (item != NULL)
		m_chaser->removeStep(item->text(KColumnNumber).toInt() - 1);

	updateStepList();
}

void ChaserEditor::slotRaiseClicked()
{
	QTreeWidgetItem* item;
	int index;

	item = m_stepList->currentItem();
	if (item != NULL)
	{
		index = m_stepList->indexOfTopLevelItem(item);

		/* Raise the step */
		m_chaser->raiseStep(index);

		/* Update step list and select the same item */
		updateStepList(index - 1);
	}
}

void ChaserEditor::slotLowerClicked()
{
	QTreeWidgetItem* item;
	int index;

	item = m_stepList->currentItem();
	if (item != NULL)
	{
		index = m_stepList->indexOfTopLevelItem(item);

		/* Raise the step */
		m_chaser->lowerStep(index);

		/* Update step list and select the same item */
		updateStepList(index + 1);
	}
}

void ChaserEditor::accept()
{
	/* Name */
	m_chaser->setName(m_nameEdit->text());

	/* Run Order */
	if (m_singleShot->isChecked() == true)
		m_chaser->setRunOrder(Chaser::SingleShot);
	else if (m_pingPong->isChecked() == true)
		m_chaser->setRunOrder(Chaser::PingPong);
	else
		m_chaser->setRunOrder(Chaser::Loop);

	/* Direction */
	if (m_backward->isChecked() == true)
		m_chaser->setDirection(Chaser::Backward);
	else
		m_chaser->setDirection(Chaser::Forward);

	/* Copy the temp chaser's contents to the original */
	m_original->copyFrom(m_chaser, false);

	_app->doc()->setModified();
	QDialog::accept();
}
