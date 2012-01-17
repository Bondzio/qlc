/*
  Q Light Controller
  fixturegroupeditor.cpp

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

#include <QTableWidgetItem>
#include <QTableWidget>
#include <QSettings>
#include <QLineEdit>
#include <QSpinBox>
#include <QDebug>

#include "fixturegroupeditor.h"
#include "fixtureselection.h"
#include "fixturegroup.h"
#include "qlcmacros.h"
#include "fixture.h"
#include "apputil.h"
#include "doc.h"

#define SETTINGS_GEOMETRY "fixturegroupeditor/geometry"

#define PROP_FIXTURE Qt::UserRole
#define PROP_HEAD Qt::UserRole + 1

FixtureGroupEditor::FixtureGroupEditor(FixtureGroup* grp, Doc* doc, QWidget* parent)
    : QDialog(parent)
    , m_original(grp)
    , m_doc(doc)
{
    Q_ASSERT(grp != NULL);
    Q_ASSERT(doc != NULL);

    setupUi(this);

    m_grp = new FixtureGroup(doc);
    m_grp->copyFrom(m_original);

    m_nameEdit->setText(m_grp->name());
    m_xSpin->setValue(m_grp->size().width());
    m_ySpin->setValue(m_grp->size().height());

    if (m_grp->displayStyle() & FixtureGroup::DisplayIcon)
        m_displayIconCheck->setChecked(true);
    if (m_grp->displayStyle() & FixtureGroup::DisplayName)
        m_displayNameCheck->setChecked(true);
    if (m_grp->displayStyle() & FixtureGroup::DisplayAddress)
        m_displayAddressCheck->setChecked(true);
    if (m_grp->displayStyle() & FixtureGroup::DisplayUniverse)
        m_displayUniverseCheck->setChecked(true);
    if (m_grp->displayStyle() & FixtureGroup::DisplayHead)
        m_displayHeadCheck->setChecked(true);

    connect(m_xSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotXSpinValueChanged(int)));
    connect(m_ySpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotYSpinValueChanged(int)));

    connect(m_displayIconCheck, SIGNAL(clicked()),
            this, SLOT(slotDisplayStyleChecked()));
    connect(m_displayNameCheck, SIGNAL(clicked()),
            this, SLOT(slotDisplayStyleChecked()));
    connect(m_displayAddressCheck, SIGNAL(clicked()),
            this, SLOT(slotDisplayStyleChecked()));
    connect(m_displayUniverseCheck, SIGNAL(clicked()),
            this, SLOT(slotDisplayStyleChecked()));
    connect(m_displayHeadCheck, SIGNAL(clicked()),
            this, SLOT(slotDisplayStyleChecked()));

    connect(m_addButton, SIGNAL(clicked()),
            this, SLOT(slotAddFixtureClicked()));
    connect(m_removeButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveFixtureClicked()));

    m_table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setResizeMode(QHeaderView::Stretch);

    updateTable();

    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());
    AppUtil::ensureWidgetIsVisible(this);
}

FixtureGroupEditor::~FixtureGroupEditor()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

void FixtureGroupEditor::accept()
{
    Q_ASSERT(m_original != NULL);
    Q_ASSERT(m_grp != NULL);

    m_grp->setName(m_nameEdit->text());

    m_original->copyFrom(m_grp);
    delete m_grp;

    QDialog::accept();
}

void FixtureGroupEditor::updateTable()
{
    // Store these since they might get reset
    int savedRow = m_row;
    int savedCol = m_column;

    disconnect(m_table, SIGNAL(cellChanged(int,int)),
               this, SLOT(slotCellChanged(int,int)));
    disconnect(m_table, SIGNAL(cellPressed(int,int)),
               this, SLOT(slotCellActivated(int,int)));

    m_table->clear();

    m_table->setRowCount(m_grp->size().height());
    m_table->setColumnCount(m_grp->size().width());

    QHashIterator <QLCPoint,GroupHead> it(m_grp->headHash());
    while (it.hasNext() == true)
    {
        it.next();

        QLCPoint pt(it.key());

        GroupHead head(it.value());
        Fixture* fxi = m_doc->fixture(head.fxi);
        if (fxi == NULL)
            continue;

        QString str;
        QIcon icon;
        if (m_grp->displayStyle() & FixtureGroup::DisplayIcon)
            icon = QIcon(":/fixture.png");
        if (m_grp->displayStyle() & FixtureGroup::DisplayName)
            str += fxi->name();
        if (((m_grp->displayStyle() & FixtureGroup::DisplayAddress)
             || (m_grp->displayStyle() & FixtureGroup::DisplayUniverse))
            && (m_grp->displayStyle() & FixtureGroup::DisplayName))
            str += "\n";
        if (m_grp->displayStyle() & FixtureGroup::DisplayAddress)
            str += QString("A:%1 ").arg(fxi->address() + 1);
        if (m_grp->displayStyle() & FixtureGroup::DisplayUniverse)
            str += QString("U:%1").arg(fxi->universe() + 1);
        if (m_grp->displayStyle() & FixtureGroup::DisplayHead)
            str += QString("H:%1").arg(head.head + 1);

        QTableWidgetItem* item = new QTableWidgetItem(icon, str);
        QFont font = item->font();
        font.setPointSize(font.pointSize() - 2);
        item->setFont(font);
        item->setData(PROP_FIXTURE, head.fxi);
        item->setData(PROP_HEAD, head.head);

        m_table->setItem(pt.y(), pt.x(), item);
    }

    connect(m_table, SIGNAL(cellPressed(int,int)),
            this, SLOT(slotCellActivated(int,int)));
    connect(m_table, SIGNAL(cellChanged(int,int)),
            this, SLOT(slotCellChanged(int,int)));

    if (savedRow < m_table->rowCount() && savedCol < m_table->columnCount())
    {
        m_row = savedRow;
        m_column = savedCol;
    }
    else
    {
        m_row = 0;
        m_column = 0;
    }

    m_table->setCurrentCell(m_row, m_column);
}

void FixtureGroupEditor::slotXSpinValueChanged(int value)
{
    m_grp->setSize(QSize(value, m_grp->size().height()));
    updateTable();
}

void FixtureGroupEditor::slotYSpinValueChanged(int value)
{
    m_grp->setSize(QSize(m_grp->size().width(), value));
    updateTable();
}

void FixtureGroupEditor::slotDisplayStyleChecked()
{
    int style = 0;
    if (m_displayIconCheck->isChecked() == true)
        style |= FixtureGroup::DisplayIcon;
    if (m_displayNameCheck->isChecked() == true)
        style |= FixtureGroup::DisplayName;
    if (m_displayAddressCheck->isChecked() == true)
        style |= FixtureGroup::DisplayAddress;
    if (m_displayUniverseCheck->isChecked() == true)
        style |= FixtureGroup::DisplayUniverse;
    if (m_displayHeadCheck->isChecked() == true)
        style |= FixtureGroup::DisplayHead;
    m_grp->setDisplayStyle(style);
    updateTable();
}

void FixtureGroupEditor::slotAddFixtureClicked()
{
    FixtureSelection fs(this, m_doc);
    fs.setMultiSelection(true);
    fs.setSelectionMode(FixtureSelection::Heads);
    fs.setDisabledHeads(m_grp->headList());
    if (fs.exec() == QDialog::Accepted)
    {
        int row = m_row;
        int col = m_column;
        foreach (GroupHead gh, fs.selectedHeads())
            m_grp->assignHead(QLCPoint(col++, row), gh);

        updateTable();
        m_table->setCurrentCell(row, col);
    }
}

void FixtureGroupEditor::slotRemoveFixtureClicked()
{
    QTableWidgetItem* item = m_table->currentItem();
    if (item == NULL)
        return;

    // In case an empty cell was selected
    QVariant var = item->data(PROP_FIXTURE);
    if (var.isValid() == false)
        return;

    m_grp->resignFixture(var.toUInt());
    delete item;
}

void FixtureGroupEditor::slotCellActivated(int row, int column)
{
    m_row = row;
    m_column = column;

    if (m_table->currentItem() == NULL)
        m_removeButton->setEnabled(false);
    else
        m_removeButton->setEnabled(true);
}

void FixtureGroupEditor::slotCellChanged(int row, int column)
{
    if (row < 0 || column < 0)
    {
        updateTable();
        return;
    }

    QHash <QLCPoint,GroupHead> hash = m_grp->headHash();
    QLCPoint from(m_column, m_row);
    QLCPoint to(column, row);
    GroupHead fromHead;
    GroupHead toHead;

    if (hash.contains(from) == true)
        fromHead = hash[from];
    if (hash.contains(to) == true)
        toHead = hash[to];

    m_grp->swap(from, to);

    updateTable();
    m_table->setCurrentCell(row, column);
    slotCellActivated(row, column);
}
