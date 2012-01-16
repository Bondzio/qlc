/*
  Q Light Controller
  fixtureselection.cpp

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
#include <QHeaderView>
#include <QLabel>

#include "qlcfixturedef.h"

#include "fixtureselection.h"
#include "fixture.h"
#include "fixture.h"
#include "doc.h"

#define KColumnName         0
#define KColumnHeads        1
#define KColumnManufacturer 2
#define KColumnModel        3
#define KColumnID           4

FixtureSelection::FixtureSelection(QWidget* parent, Doc* doc)
    : QDialog(parent)
    , m_doc(doc)
{
    Q_ASSERT(doc != NULL);

    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotItemDoubleClicked()));

    connect(m_tree, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSelectionChanged()));
}

FixtureSelection::~FixtureSelection()
{
}

int FixtureSelection::exec()
{
    fillTree();
    return QDialog::exec();
}

/****************************************************************************
 * Selected fixtures
 ****************************************************************************/

QList <quint32> FixtureSelection::selection() const
{
    return m_selection;
}

/****************************************************************************
 * Multi-selection
 ****************************************************************************/

void FixtureSelection::setMultiSelection(bool multi)
{
    if (multi == true)
        m_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    else
        m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
}

/****************************************************************************
 * Disabled fixtures
 ****************************************************************************/

void FixtureSelection::setDisabledFixtures(const QList <quint32>& disabled)
{
    m_disabledFixtures = disabled;
}

/****************************************************************************
 * Tree
 ****************************************************************************/

void FixtureSelection::fillTree()
{
    m_tree->clear();

    foreach (Fixture* fixture, m_doc->fixtures())
    {
        Q_ASSERT(fixture != NULL);
        QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
        item->setText(KColumnName, fixture->name());
        item->setText(KColumnHeads, QString::number(fixture->heads()));
        item->setText(KColumnID, QString::number(fixture->id()));

        if (fixture->fixtureDef() == NULL)
        {
            item->setText(KColumnManufacturer, tr("Generic"));
            item->setText(KColumnModel, tr("Generic"));
        }
        else
        {
            item->setText(KColumnManufacturer, fixture->fixtureDef()->manufacturer());
            item->setText(KColumnModel, fixture->fixtureDef()->model());
        }

        if (m_disabledFixtures.contains(fixture->id()) == true)
            item->setFlags(0); // Disables the item
    }

    if (m_tree->topLevelItemCount() == 0)
    {
        m_tree->setHeaderLabel(tr("No fixtures available"));
        m_tree->header()->hideSection(KColumnManufacturer);
        m_tree->header()->hideSection(KColumnModel);
        QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
        item->setText(0, tr("Go to the Fixture Manager and add some fixtures first."));
        m_tree->setEnabled(false);
        m_buttonBox->setStandardButtons(QDialogButtonBox::Cancel);
    }
    else
    {
        m_tree->sortItems(KColumnName, Qt::AscendingOrder);
        m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);
        m_buttonBox->setStandardButtons(QDialogButtonBox::Cancel);
    }
}

void FixtureSelection::slotItemDoubleClicked()
{
    if (m_tree->selectedItems().isEmpty() == false)
        accept();
}

void FixtureSelection::slotSelectionChanged()
{
    if (m_tree->selectedItems().size() > 0)
        m_buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    else
        m_buttonBox->setStandardButtons(QDialogButtonBox::Cancel);
}

void FixtureSelection::accept()
{
    m_selection.clear();

    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    while (it.hasNext() == true)
        m_selection << it.next()->text(KColumnID).toUInt();

    QDialog::accept();
}
