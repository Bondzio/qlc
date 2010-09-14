/*
  Q Light Controller
  shuffleeditor.cpp

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
#include <QHeaderView>
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
#include "shuffleeditor.h"
#include "fixture.h"
#include "shuffle.h"
#include "app.h"
#include "doc.h"

extern App* _app;

#define KColumnNumber     0
#define KColumnFunction   1
#define KColumnFunctionID 2

ShuffleEditor::ShuffleEditor(QWidget* parent, Shuffle* shuffle) : QDialog(parent)
{
	Q_ASSERT(shuffle != NULL);

	setupUi(this);

	/* Resize columns to fit contents */
	m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);

	/* Connect UI controls */
	connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
		this, SLOT(slotNameEdited(const QString&)));
	connect(m_add, SIGNAL(clicked()), this, SLOT(slotAddClicked()));
	connect(m_remove, SIGNAL(clicked()), this, SLOT(slotRemoveClicked()));
	
	/* Create a copy of the original shuffle so that we can freely modify
	   it and keep a pointer to the original so that we can move the
	   contents from the copied shuffle to the original when OK is clicked */
	m_shuffle = new Shuffle(this);
	m_shuffle->copyFrom(shuffle);
	Q_ASSERT(m_shuffle != NULL);
	m_original = shuffle;

	/* Name edit */
	m_nameEdit->setText(m_shuffle->name());
	m_nameEdit->setSelection(0, m_nameEdit->text().length());
	setWindowTitle(tr("Shuffle - %1").arg(m_shuffle->name()));

	/* Bus */
	connect(m_busCombo, SIGNAL(activated(int)),
		this, SLOT(slotBusComboActivated(int)));
	fillBusCombo();

	/* Running direction */
	switch (m_shuffle->direction())
	{
	default:
	case Shuffle::Forward:
		m_forward->setChecked(true);
		break;
	case Shuffle::Backward:
		m_backward->setChecked(true);
		break;
	}

	/* Shuffle steps */
	updateStepList(0);
}

ShuffleEditor::~ShuffleEditor()
{
	delete m_shuffle;
}

void ShuffleEditor::fillBusCombo()
{
	m_busCombo->clear();
	m_busCombo->addItems(Bus::instance()->idNames());
	m_busCombo->setCurrentIndex(m_shuffle->busID());
}

void ShuffleEditor::updateStepList(int selectIndex)
{
	m_tree->clear();

	QListIterator <t_function_id> it(*m_shuffle->sceneSteps());
	while (it.hasNext() == true)
	{
		QTreeWidgetItem* item;
		Function* function;
		t_function_id fid;
		QString str;

		fid = it.next();
		function = _app->doc()->function(fid);
		Q_ASSERT(function != NULL);

		item = new QTreeWidgetItem(m_tree);
		item->setText(KColumnNumber, "###");
		item->setText(KColumnFunction, function->name());
		item->setText(KColumnFunctionID, str.setNum(fid));
	}

	/* Select the specified item */
	m_tree->setCurrentItem(m_tree->topLevelItem(selectIndex));

	/* Update the order number column */
	updateOrderNumbers();
}

void ShuffleEditor::updateOrderNumbers()
{
	int i = 1;
	QString num;

	QTreeWidgetItemIterator it(m_tree);
	while (*it != NULL)
	{
		num.sprintf("%.03d", i++);
		(*it)->setText(KColumnNumber, num);
		++it;
	}
}

void ShuffleEditor::slotNameEdited(const QString& text)
{
	setWindowTitle(QString(tr("Shuffle editor - %1")).arg(text));
}

void ShuffleEditor::slotBusComboActivated(int index)
{
	Q_ASSERT(m_shuffle != NULL);
	m_shuffle->setBus(index);
}

void ShuffleEditor::slotAddClicked()
{
	FunctionSelection fs(this, true, m_original->id());
	if (fs.exec() == QDialog::Accepted)
	{
		QListIterator <t_function_id> it(fs.selection());
		while (it.hasNext() == true)
			m_shuffle->addStep(it.next());

		// Update all steps in the list
		updateStepList();
	}
}

void ShuffleEditor::slotRemoveClicked()
{
	QTreeWidgetItem* item = m_tree->currentItem();
	if (item != NULL)
	{
		int index = item->text(KColumnNumber).toInt() - 1;
		m_shuffle->removeStep(index);
		updateStepList(index - 1);
	}
}

void ShuffleEditor::accept()
{
	/* Name */
	m_shuffle->setName(m_nameEdit->text());

	/* Direction */
	if (m_backward->isChecked() == true)
		m_shuffle->setDirection(Shuffle::Backward);
	else
		m_shuffle->setDirection(Shuffle::Forward);

	/* Copy the temp shuffle's contents to the original */
	m_original->copyFrom(m_shuffle);

	/* Mark doc as modified, close and accept */
	_app->doc()->setModified();
	QDialog::accept();
}
