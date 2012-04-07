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
#include <QSplitter>
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

#define KColumnUniverse   0
#define KColumnPlugin     1
#define KColumnOutputName 2
#define KColumnOutput     3

#define SETTINGS_SPLITTER "outputmanager/splitter"

OutputManager* OutputManager::s_instance = NULL;

/****************************************************************************
 * Initialization
 ****************************************************************************/

OutputManager::OutputManager(QWidget* parent, OutputMap* outputMap, Qt::WindowFlags flags)
    : QWidget(parent, flags)
    , m_outputMap(outputMap)
{
    Q_ASSERT(outputMap != NULL);

    new QVBoxLayout(this);
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->setSpacing(0);

    m_splitter = new QSplitter(this);
    layout()->addWidget(m_splitter);

    /* Tree */
    m_tree = new QTreeWidget(this);
    m_splitter->addWidget(m_tree);
    m_tree->setRootIsDecorated(false);
    m_tree->setItemsExpandable(false);
    m_tree->setSortingEnabled(false);
    m_tree->setAllColumnsShowFocus(true);
    m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);

    QStringList columns;
    columns << tr("Universe") << tr("Plugin") << tr("Output");
    m_tree->setHeaderLabels(columns);

    connect(m_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(slotEditClicked()));

    connect(m_outputMap, SIGNAL(pluginConfigurationChanged(const QString&)),
            this, SLOT(updateTree()));

    updateTree();
    m_tree->setCurrentItem(m_tree->topLevelItem(0));
    slotEditClicked();

    QSettings settings;
    QVariant var = settings.value(SETTINGS_SPLITTER);
    if (var.isValid() == true)
        m_splitter->restoreState(var.toByteArray());
}

OutputManager::~OutputManager()
{
    QSettings settings;
    settings.setValue(SETTINGS_SPLITTER, m_splitter->saveState());

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

void OutputManager::slotEditClicked()
{
    QTreeWidgetItem* item = m_tree->currentItem();
    if (item == NULL)
        return;

    if (currentEditor() != NULL)
        delete currentEditor();

    quint32 universe = item->text(KColumnUniverse).toUInt() - 1;
    QWidget* editor = new OutputPatchEditor(this, universe, m_outputMap);
    m_splitter->addWidget(editor);
    connect(editor, SIGNAL(mappingChanged()), this, SLOT(updateTree()));
    editor->show();
}

QWidget* OutputManager::currentEditor() const
{
    Q_ASSERT(m_splitter != NULL);
    if (m_splitter->count() < 2)
        return NULL;
    else
        return m_splitter->widget(1);
}
