/*
  Q Light Controller
  outputmanager.cpp

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
#include <QMdiSubWindow>
#include <QTreeWidget>
#include <QStringList>
#include <QHeaderView>
#include <QSettings>
#include <QMdiArea>
#include <QToolBar>
#include <QAction>
#include <QDebug>
#include <QMenu>

#include "qlcoutplugin.h"
#include "outputpatcheditor.h"
#include "outputmanager.h"
#include "outputpatch.h"
#include "outputmap.h"
#include "apputil.h"
#include "app.h"

#define KColumnUniverse   0
#define KColumnPlugin     1
#define KColumnOutputName 2
#define KColumnOutput     3

OutputManager* OutputManager::s_instance = NULL;

/****************************************************************************
 * Initialization
 ****************************************************************************/

OutputManager::OutputManager(QWidget* parent, OutputMap* outputMap, Qt::WindowFlags flags)
    : QWidget(parent, flags)
    , m_outputMap(outputMap)
{
    Q_ASSERT(outputMap != NULL);

    /* Create a new layout for this widget */
    new QVBoxLayout(this);
    layout()->setMargin(1);
    layout()->setSpacing(1);

    /* Toolbar */
    m_toolbar = new QToolBar(tr("Output Manager"), this);
    m_toolbar->addAction(QIcon(":/edit.png"), tr("Edit Mapping"),
                         this, SLOT(slotEditClicked()));
    layout()->addWidget(m_toolbar);

    /* Tree */
    m_tree = new QTreeWidget(this);
    layout()->addWidget(m_tree);
    m_tree->setRootIsDecorated(false);
    m_tree->setItemsExpandable(false);
    m_tree->setSortingEnabled(false);
    m_tree->setAllColumnsShowFocus(true);
    m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);

    QStringList columns;
    columns << tr("Universe") << tr("Plugin") << tr("Output");
    m_tree->setHeaderLabels(columns);

    connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotEditClicked()));

    connect(m_outputMap, SIGNAL(pluginConfigurationChanged(const QString&)),
            this, SLOT(slotPluginConfigurationChanged()));

    updateTree();
}

OutputManager::~OutputManager()
{
    OutputManager::s_instance = NULL;
}

OutputManager* OutputManager::instance()
{
    return s_instance;
}

void OutputManager::createAndShow(QWidget* parent, OutputMap* outputMap)
{
    /* Must not create more than one instance */
    Q_ASSERT(s_instance == NULL);

    QMdiArea* area = qobject_cast<QMdiArea*> (parent);
    Q_ASSERT(area != NULL);
    QMdiSubWindow* sub = new QMdiSubWindow;
    s_instance = new OutputManager(sub, outputMap);
    sub->setWidget(s_instance);
    QWidget* window = area->addSubWindow(sub);

    /* Set some common properties for the window and show it */
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setWindowIcon(QIcon(":/output.png"));
    window->setWindowTitle(tr("Outputs"));
    window->setContextMenuPolicy(Qt::CustomContextMenu);

    sub->setSystemMenu(NULL);
}

/*****************************************************************************
 * Tree widget
 *****************************************************************************/

void OutputManager::updateTree()
{
    m_tree->clear();
    for (quint32 uni = 0; uni < m_outputMap->universes(); uni++)
        updateItem(new QTreeWidgetItem(m_tree), uni);
}

void OutputManager::updateItem(QTreeWidgetItem* item, quint32 universe)
{
    Q_ASSERT(item != NULL);

    OutputPatch* op = m_outputMap->patch(universe);
    Q_ASSERT(op != NULL);

    item->setText(KColumnUniverse, QString::number(universe + 1));
    item->setText(KColumnPlugin, op->pluginName());
    item->setText(KColumnOutputName, op->outputName());
    item->setText(KColumnOutput, QString::number(op->output() + 1));
}

void OutputManager::slotPluginConfigurationChanged()
{
    updateTree();
}

/****************************************************************************
 * Toolbar
 ****************************************************************************/

void OutputManager::slotEditClicked()
{
    QTreeWidgetItem* item = m_tree->currentItem();
    if (item == NULL)
        return;

    quint32 universe = item->text(KColumnUniverse).toUInt() - 1;
    OutputPatchEditor ope(this, universe, m_outputMap);
    if (ope.exec() == QDialog::Accepted)
        updateTree();
}
