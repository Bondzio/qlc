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
#include "fixturegroup.h"
#include "qlcmacros.h"
#include "fixture.h"
#include "apputil.h"
#include "doc.h"

#define SETTINGS_GEOMETRY "fixturegroupeditor/geometry"

#define PROP_FIXTURE Qt::UserRole

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

    m_table->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

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
    m_grp->setSize(QSize(m_xSpin->value(), m_ySpin->value()));

    m_original->copyFrom(m_grp);
    delete m_grp;

    QDialog::accept();
}

void FixtureGroupEditor::updateTable()
{
    disconnect(m_table, SIGNAL(cellChanged(int,int)),
               this, SLOT(slotCellChanged(int,int)));
    disconnect(m_table, SIGNAL(cellPressed(int,int)),
               this, SLOT(slotCellActivated(int,int)));

    m_table->clear();

    m_table->setRowCount(m_grp->size().height());
    m_table->setColumnCount(m_grp->size().width());

    QHashIterator <QLCPoint,quint32> it(m_grp->fixtureHash());
    while (it.hasNext() == true)
    {
        it.next();

        QLCPoint pt(it.key());
        Fixture* fxi = m_doc->fixture(it.value());
        if (fxi == NULL)
            continue;

        QTableWidgetItem* item = new QTableWidgetItem;
        QFont font = item->font();
        font.setPointSize(10);
        item->setFont(font);
        item->setData(PROP_FIXTURE, it.value());

        QString str;
        if (m_grp->displayStyle() & FixtureGroup::DisplayIcon)
            item->setIcon(QIcon(":/fixture.png"));
        if (m_grp->displayStyle() & FixtureGroup::DisplayName)
            str += fxi->name();
        if ((m_grp->displayStyle() & FixtureGroup::DisplayAddress
             || m_grp->displayStyle() & FixtureGroup::DisplayUniverse)
            && m_grp->displayStyle() & FixtureGroup::DisplayName)
            str += "\n";
        if (m_grp->displayStyle() & FixtureGroup::DisplayAddress)
            str += QString("DMX:%1 ").arg(fxi->address() + 1);
        if (m_grp->displayStyle() & FixtureGroup::DisplayUniverse)
            str += QString("U:%1").arg(fxi->universe() + 1);
        item->setText(str);

        m_table->setItem(pt.y(), pt.x(), item);
    }

    connect(m_table, SIGNAL(cellPressed(int,int)),
            this, SLOT(slotCellActivated(int,int)));
    connect(m_table, SIGNAL(cellChanged(int,int)),
            this, SLOT(slotCellChanged(int,int)));

    m_row = -1;
    m_column = -1;
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
    m_grp->setDisplayStyle(style);
    updateTable();
}

void FixtureGroupEditor::slotCellActivated(int row, int column)
{
    m_row = row;
    m_column = column;
}

void FixtureGroupEditor::slotCellChanged(int row, int column)
{
    if (row < 0 || column < 0)
        return;

    QHash <QLCPoint,quint32> hash = m_grp->fixtureHash();
    QLCPoint from(m_column, m_row);
    QLCPoint to(column, row);
    quint32 fromId = Fixture::invalidId();
    quint32 toId = Fixture::invalidId();

    if (hash.contains(from) == true)
        fromId = hash[from];
    if (hash.contains(to) == true)
        toId = hash[to];

    m_grp->swap(from, to);

    updateTable();
    m_table->setCurrentCell(row, column);
    slotCellActivated(row, column);
}
