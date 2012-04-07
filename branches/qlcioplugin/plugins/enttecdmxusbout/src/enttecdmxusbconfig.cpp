/*
  Q Light Controller
  enttecdmxusbconfig.cpp

  Copyright (C) Heikki Junnila

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <QPushButton>
#include <QHeaderView>
#include <QTreeWidget>
#include <QSettings>
#include <QLayout>
#include <QDebug>
#include <QTimer>

#include "enttecdmxusbconfig.h"
#include "enttecdmxusbwidget.h"
#include "enttecdmxusbout.h"

#define SETTINGS_GEOMETRY "enttecdmxusbconfig/geometry"

#define COL_NAME   0
#define COL_SERIAL 1
#define COL_OPEN   2
#define COL_PRO    3
#define PROP_WIDGET Qt::UserRole

EnttecDMXUSBConfig::EnttecDMXUSBConfig(EnttecDMXUSBOut* plugin, QWidget* parent)
    : QDialog(parent)
    , m_plugin(plugin)
    , m_tree(new QTreeWidget(this))
    , m_refreshButton(new QPushButton(tr("Refresh"), this))
    , m_closeButton(new QPushButton(tr("Close"), this))
    , m_ignoreItemChanged(false)
{
    Q_ASSERT(plugin != NULL);

    setWindowTitle(plugin->name());

    QStringList header;
    header << tr("Name") << tr("Serial") << QString("Open") << QString("Pro");
    m_tree->setHeaderLabels(header);
    m_tree->setSelectionMode(QAbstractItemView::NoSelection);
    m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);

    QVBoxLayout* vbox = new QVBoxLayout(this);
    vbox->addWidget(m_tree);

    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addWidget(m_refreshButton);
    hbox->addStretch();
    hbox->addWidget(m_closeButton);
    vbox->addLayout(hbox);

    connect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotItemChanged(QTreeWidgetItem*,int)));
    connect(m_refreshButton, SIGNAL(clicked()), this, SLOT(slotRefresh()));
    connect(m_closeButton, SIGNAL(clicked()), this, SLOT(accept()));

    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());

    slotRefresh();
}

EnttecDMXUSBConfig::~EnttecDMXUSBConfig()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

void EnttecDMXUSBConfig::slotItemChanged(QTreeWidgetItem* item, int column)
{
    if (m_ignoreItemChanged == true)
        return;

    m_ignoreItemChanged = true;

    if (column == COL_OPEN)
    {
        if (item->checkState(column) == Qt::Checked)
            item->setCheckState(COL_PRO, Qt::Unchecked);
        else
            item->setCheckState(COL_PRO, Qt::Checked);
    }
    else
    {
        if (item->checkState(column) == Qt::Checked)
            item->setCheckState(COL_OPEN, Qt::Unchecked);
        else
            item->setCheckState(COL_OPEN, Qt::Checked);
    }

    QMap <QString,QVariant> typeMap(QLCFTDI::typeMap());
    if (item->checkState(COL_OPEN) == Qt::Checked)
        typeMap[item->text(COL_SERIAL)] = EnttecDMXUSBWidget::Open;
    else
        typeMap[item->text(COL_SERIAL)] = EnttecDMXUSBWidget::Pro;
    QLCFTDI::storeTypeMap(typeMap);

    m_ignoreItemChanged = false;

    QTimer::singleShot(0, this, SLOT(slotRefresh()));
}

void EnttecDMXUSBConfig::slotRefresh()
{
    m_plugin->rescanWidgets();

    m_ignoreItemChanged = true;

    m_tree->clear();
    QListIterator <EnttecDMXUSBWidget*> it(m_plugin->widgets());
    while (it.hasNext() == true)
    {
        EnttecDMXUSBWidget* widget = it.next();
        QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
        item->setText(COL_NAME, widget->name());
        item->setText(COL_SERIAL, widget->serial());
        if (widget->type() == EnttecDMXUSBWidget::Open)
        {
            item->setCheckState(COL_OPEN, Qt::Checked);
            item->setCheckState(COL_PRO, Qt::Unchecked);
        }
        else
        {
            item->setCheckState(COL_OPEN, Qt::Unchecked);
            item->setCheckState(COL_PRO, Qt::Checked);
        }
    }

    m_ignoreItemChanged = false;
}
