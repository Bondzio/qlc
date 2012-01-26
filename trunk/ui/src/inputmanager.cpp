/*
  Q Light Controller
  inputmanager.cpp

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
#include <QHeaderView>
#include <QStringList>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QSettings>
#include <QSplitter>
#include <QMdiArea>
#include <QAction>
#include <QTimer>
#include <QDebug>
#include <QIcon>

#include "inputpatcheditor.h"
#include "inputmanager.h"
#include "qlcinplugin.h"
#include "inputpatch.h"
#include "inputmap.h"
#include "apputil.h"
#include "doc.h"

#define KColumnUniverse 0
#define KColumnPlugin   1
#define KColumnInput    2
#define KColumnProfile  3
#define KColumnEditor   4
#define KColumnInputNum 5

#define SETTINGS_SPLITTER "inputmanager/splitter"

InputManager* InputManager::s_instance = NULL;

/****************************************************************************
 * Initialization
 ****************************************************************************/

InputManager::InputManager(QWidget* parent, InputMap* inputMap, Qt::WindowFlags flags)
    : QWidget(parent, flags)
    , m_inputMap(inputMap)
{
    Q_ASSERT(inputMap != NULL);

    /* Create a new layout for this widget */
    new QVBoxLayout(this);
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->setSpacing(0);

    m_splitter = new QSplitter(Qt::Horizontal, this);
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
    columns << tr("Universe") << tr("Plugin") << tr("Input") << tr("Profile")
            << tr("Editor universe");
    m_tree->setHeaderLabels(columns);

    connect(m_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(slotEditClicked()));

    /* Timer that clears the input data icon after a while */
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimerTimeout()));

    /* Listen to input map's input data signals */
    connect(m_inputMap, SIGNAL(inputValueChanged(quint32,quint32,uchar)),
            this, SLOT(slotInputValueChanged(quint32,quint32,uchar)));

    /* Listen to plugin configuration changes */
    connect(m_inputMap, SIGNAL(pluginConfigurationChanged(const QString&)),
            this, SLOT(updateTree()));

    updateTree();
    m_tree->setCurrentItem(m_tree->topLevelItem(0));
    slotEditClicked();

    QSettings settings;
    QVariant var = settings.value(SETTINGS_SPLITTER);
    if (var.isValid() == true)
        m_splitter->restoreState(var.toByteArray());
}

InputManager::~InputManager()
{
    QSettings settings;
    settings.setValue(SETTINGS_SPLITTER, m_splitter->saveState());

    InputManager::s_instance = NULL;
}

InputManager* InputManager::instance()
{
    return s_instance;
}

void InputManager::createAndShow(QWidget* parent, InputMap* inputMap)
{
    /* Must not create more than one instance */
    Q_ASSERT(s_instance == NULL);

    QMdiArea* area = qobject_cast<QMdiArea*> (parent);
    Q_ASSERT(area != NULL);
    QMdiSubWindow* sub = new QMdiSubWindow;
    s_instance = new InputManager(sub, inputMap);
    sub->setWidget(s_instance);
    QWidget* window = area->addSubWindow(sub);

    /* Set some common properties for the window and show it */
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setWindowIcon(QIcon(":/input.png"));
    window->setWindowTitle(tr("Inputs"));
    window->setContextMenuPolicy(Qt::CustomContextMenu);

    sub->setSystemMenu(NULL);
}

/*****************************************************************************
 * Tree widget
 *****************************************************************************/

void InputManager::updateTree()
{
    m_tree->clear();
    for (quint32 i = 0; i < m_inputMap->universes(); i++)
    {
        InputPatch* inputPatch = m_inputMap->patch(i);
        Q_ASSERT(inputPatch != NULL);

        QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
        updateItem(item, inputPatch, i);
    }
}

void InputManager::updateItem(QTreeWidgetItem* item, InputPatch* ip,
                              quint32 universe)
{
    Q_ASSERT(item != NULL);
    Q_ASSERT(ip != NULL);

    item->setText(KColumnUniverse, QString("%1").arg(universe + 1));
    item->setText(KColumnPlugin, ip->pluginName());
    item->setText(KColumnInput, ip->inputName());
    item->setText(KColumnProfile, ip->profileName());
    if (m_inputMap->editorUniverse() == universe)
    {
        item->setCheckState(KColumnEditor, Qt::Checked);
        item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);
    }
    item->setText(KColumnInputNum, QString("%1").arg(ip->input() + 1));
}

QWidget* InputManager::currentEditor() const
{
    Q_ASSERT(m_splitter != NULL);
    if (m_splitter->count() < 2)
        return NULL;
    else
        return m_splitter->widget(1);
}

void InputManager::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    Q_UNUSED(channel);
    Q_UNUSED(value);

    QTreeWidgetItem* item = m_tree->topLevelItem(universe);
    if (item == NULL)
        return;

    /* Show an icon on a universe row that received input data */
    QIcon icon(":/input.png");
    item->setIcon(KColumnUniverse, icon);

    /* Restart the timer */
    m_timer->start(250);
}

void InputManager::slotTimerTimeout()
{
    QTreeWidgetItemIterator it(m_tree);
    while (*it != NULL)
    {
        (*it)->setIcon(KColumnUniverse, QIcon());
        ++it;
    }
}

void InputManager::slotEditClicked()
{
    QTreeWidgetItem* item = m_tree->currentItem();
    if (item == NULL)
        return;

    if (currentEditor() != NULL)
        delete currentEditor();

    quint32 universe = item->text(KColumnUniverse).toInt() - 1;
    QWidget* editor = new InputPatchEditor(this, universe, m_inputMap);
    m_splitter->addWidget(editor);
    connect(editor, SIGNAL(mappingChanged()), this, SLOT(slotMappingChanged()));
    editor->show();
}

void InputManager::slotMappingChanged()
{
    QTreeWidgetItem* item = m_tree->currentItem();
    Q_ASSERT(item != NULL);

    uint universe = item->text(KColumnUniverse).toUInt() - 1;

    InputPatch* inputPatch = m_inputMap->patch(universe);
    Q_ASSERT(inputPatch != NULL);

    updateItem(item, inputPatch, universe);
}
