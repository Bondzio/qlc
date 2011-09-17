/*
  Q Light Controller
  functionselection.cpp

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
#include <QMessageBox>
#include <QToolBar>
#include <QDebug>

#include "functionselection.h"
#include "collectioneditor.h"
#include "chasereditor.h"
#include "scripteditor.h"
#include "sceneeditor.h"
#include "mastertimer.h"
#include "collection.h"
#include "outputmap.h"
#include "efxeditor.h"
#include "inputmap.h"
#include "function.h"
#include "fixture.h"
#include "chaser.h"
#include "script.h"
#include "scene.h"
#include "efx.h"
#include "doc.h"

#define KColumnName 0
#define KColumnType 1
#define KColumnID   2

/*****************************************************************************
 * Initialization
 *****************************************************************************/

FunctionSelection::FunctionSelection(QWidget* parent, Doc* doc, OutputMap* outputMap,
                                     InputMap* inputMap, MasterTimer* masterTimer)
    : QDialog(parent)
    , m_doc(doc)
    , m_outputMap(outputMap)
    , m_inputMap(inputMap)
    , m_masterTimer(masterTimer)
    , m_multiSelection(true)
    , m_filter(Function::Scene | Function::Chaser | Function::Collection | Function::EFX | Function::Script)
    , m_constFilter(false)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(outputMap != NULL);
    Q_ASSERT(inputMap != NULL);
    Q_ASSERT(masterTimer != NULL);

    m_toolbar = NULL;
    m_addSceneAction = NULL;
    m_addChaserAction = NULL;
    m_addEFXAction = NULL;
    m_addCollectionAction = NULL;

    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    /* Create toolbar */
    initToolBar();

    connect(m_sceneCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotSceneChecked(bool)));

    connect(m_chaserCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotChaserChecked(bool)));

    connect(m_efxCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotEFXChecked(bool)));

    connect(m_collectionCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotCollectionChecked(bool)));

    connect(m_scriptCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotScriptChecked(bool)));
}

int FunctionSelection::exec()
{
    m_sceneCheck->setChecked(m_filter & Function::Scene);
    m_addSceneAction->setEnabled(m_filter & Function::Scene);

    m_chaserCheck->setChecked(m_filter & Function::Chaser);
    m_addChaserAction->setEnabled(m_filter & Function::Chaser);

    m_efxCheck->setChecked(m_filter & Function::EFX);
    m_addEFXAction->setEnabled(m_filter & Function::EFX);

    m_collectionCheck->setChecked(m_filter & Function::Collection);
    m_addCollectionAction->setEnabled(m_filter & Function::Collection);

    m_scriptCheck->setChecked(m_filter & Function::Script);
    m_addScriptAction->setEnabled(m_filter & Function::Script);

    if (m_constFilter == true)
    {
        m_sceneCheck->setEnabled(false);
        m_chaserCheck->setEnabled(false);
        m_efxCheck->setEnabled(false);
        m_collectionCheck->setEnabled(false);
        m_scriptCheck->setEnabled(false);
    }

    /* Multiple/single selection */
    if (m_multiSelection == true)
        m_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    else
        m_tree->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(m_tree, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotItemSelectionChanged()));
    connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotItemDoubleClicked(QTreeWidgetItem*)));

    refillTree();
    m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);

    slotItemSelectionChanged();

    return QDialog::exec();
}

FunctionSelection::~FunctionSelection()
{
}

/*****************************************************************************
 * Multi-selection
 *****************************************************************************/

void FunctionSelection::setMultiSelection(bool multi)
{
    m_multiSelection = multi;
}

/*****************************************************************************
 * Filter
 *****************************************************************************/

void FunctionSelection::setFilter(int types, bool constFilter)
{
    m_filter = types;
    m_constFilter = constFilter;
}

/*****************************************************************************
 * Disabled functions
 *****************************************************************************/

void FunctionSelection::setDisabledFunctions(const QList <quint32>& ids)
{
    m_disabledFunctions = ids;
}

QList <quint32> FunctionSelection::disabledFunctions() const
{
    return m_disabledFunctions;
}

/*****************************************************************************
 * Selection
 *****************************************************************************/

const QList <quint32> FunctionSelection::selection() const
{
    return m_selection;
}

/*****************************************************************************
 * Toolbar
 *****************************************************************************/

void FunctionSelection::initToolBar()
{
    m_toolbar = new QToolBar(this);
    layout()->setMenuBar(m_toolbar);

    m_addSceneAction = m_toolbar->addAction(QIcon(":/scene.png"),
                                            tr("New Scene"), this, SLOT(slotNewScene()));
    m_addChaserAction = m_toolbar->addAction(QIcon(":/chaser.png"),
                        tr("New Chaser"), this, SLOT(slotNewChaser()));
    m_addEFXAction = m_toolbar->addAction(QIcon(":/efx.png"),
                                          tr("New EFX"), this, SLOT(slotNewEFX()));
    m_addCollectionAction = m_toolbar->addAction(QIcon(":/collection.png"),
                            tr("New Collection"), this, SLOT(slotNewCollection()));
    m_addScriptAction = m_toolbar->addAction(QIcon(":/script.png"),
                            tr("New Script"), this, SLOT(slotNewScript()));
}

void FunctionSelection::slotNewScene()
{
    Function* function = new Scene(m_doc);
    if (m_doc->addFunction(function) == true)
        addFunction(function);
    else
        addFunctionErrorMessage();
}

void FunctionSelection::slotNewChaser()
{
    Function* function = new Chaser(m_doc);
    if (m_doc->addFunction(function) == true)
        addFunction(function);
    else
        addFunctionErrorMessage();
}

void FunctionSelection::slotNewEFX()
{
    Function* function = new EFX(m_doc);
    if (m_doc->addFunction(function) == true)
        addFunction(function);
    else
        addFunctionErrorMessage();
}

void FunctionSelection::slotNewCollection()
{
    Function* function = new Collection(m_doc);
    if (m_doc->addFunction(function) == true)
        addFunction(function);
    else
        addFunctionErrorMessage();
}

void FunctionSelection::slotNewScript()
{
    Function* function = new Script(m_doc);
    if (m_doc->addFunction(function) == true)
        addFunction(function);
    else
        addFunctionErrorMessage();
}

/*****************************************************************************
 * Tree
 *****************************************************************************/

void FunctionSelection::addFunction(Function* function)
{
    QTreeWidgetItem* item;

    Q_ASSERT(function != NULL);

    /* Create a new item for the function */
    item = new QTreeWidgetItem(m_tree);
    updateFunctionItem(item, function);

    /* Append the new function to current selection */
    item->setSelected(true);

    if (editFunction(function) == QDialog::Rejected)
    {
        m_doc->deleteFunction(function->id());
        delete item;
    }
    else
    {
        updateFunctionItem(item, function);
        m_tree->sortItems(KColumnName, Qt::AscendingOrder);
        m_tree->scrollToItem(item);
        m_tree->setCurrentItem(item);
    }
}

void FunctionSelection::addFunctionErrorMessage()
{
    QMessageBox::critical(this, tr("Function creation failed"),
                          tr("Unable to create new function."));
}

void FunctionSelection::updateFunctionItem(QTreeWidgetItem* item, Function* function)
{
    item->setText(KColumnName, function->name());
    item->setText(KColumnType, function->typeString());
    item->setText(KColumnID, QString::number(function->id()));
}

void FunctionSelection::refillTree()
{
    m_tree->clear();

    /* Fill the tree */
    foreach (Function* function, m_doc->functions())
    {
        if (m_filter & function->type())
        {
            QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
            updateFunctionItem(item, function);

            if (disabledFunctions().contains(function->id()))
                item->setFlags(0); // Disables the item
        }
    }
}

void FunctionSelection::slotItemSelectionChanged()
{
    QList <quint32> removeList(m_selection);

    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    while (it.hasNext() == true)
    {
        quint32 id = it.next()->text(KColumnID).toInt();
        if (m_selection.contains(id) == false)
            m_selection.append(id);

        removeList.removeAll(id);
    }

    while (removeList.isEmpty() == false)
        m_selection.removeAll(removeList.takeFirst());

    if (m_selection.isEmpty() == true)
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    else
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void FunctionSelection::slotItemDoubleClicked(QTreeWidgetItem* item)
{
    if (item == NULL)
        return;

    accept();
}

void FunctionSelection::slotSceneChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::Scene);
    else
        m_filter = (m_filter & ~Function::Scene);
    m_addSceneAction->setEnabled(state);
    refillTree();
}

void FunctionSelection::slotChaserChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::Chaser);
    else
        m_filter = (m_filter & ~Function::Chaser);
    m_addChaserAction->setEnabled(state);
    refillTree();
}

void FunctionSelection::slotEFXChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::EFX);
    else
        m_filter = (m_filter & ~Function::EFX);
    m_addEFXAction->setEnabled(state);
    refillTree();
}

void FunctionSelection::slotCollectionChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::Collection);
    else
        m_filter = (m_filter & ~Function::Collection);
    m_addCollectionAction->setEnabled(state);
    refillTree();
}

void FunctionSelection::slotScriptChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::Script);
    else
        m_filter = (m_filter & ~Function::Script);
    m_addScriptAction->setEnabled(state);
    refillTree();
}

/*****************************************************************************
 * Helpers
 *****************************************************************************/

int FunctionSelection::editFunction(Function* function)
{
    int result = QDialog::Rejected;

    Q_ASSERT(function != NULL);

    if (function->type() == Function::Scene)
    {
        SceneEditor editor(this, qobject_cast<Scene*> (function), m_doc, m_outputMap,
                           m_inputMap, m_masterTimer);
        result = editor.exec();
    }
    else if (function->type() == Function::Chaser)
    {
        ChaserEditor editor(this, qobject_cast<Chaser*> (function), m_doc, m_outputMap,
                            m_inputMap, m_masterTimer);
        result = editor.exec();
    }
    else if (function->type() == Function::Collection)
    {
        CollectionEditor editor(this, qobject_cast<Collection*> (function), m_doc,
                                m_outputMap, m_inputMap, m_masterTimer);
        result = editor.exec();
    }
    else if (function->type() == Function::EFX)
    {
        EFXEditor editor(this, qobject_cast<EFX*> (function), m_doc);
        result = editor.exec();
    }
    else if (function->type() == Function::Script)
    {
        ScriptEditor editor(this, qobject_cast<Script*> (function), m_doc, m_outputMap,
                            m_inputMap, m_masterTimer);
        result = editor.exec();
    }
    else
    {
        result = QDialog::Rejected;
    }

    return result;
}
