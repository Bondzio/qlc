/*
  Q Light Controller
  functionmanager.cpp

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QTreeWidgetItemIterator>
#include <QTreeWidgetItem>
#include <QMdiSubWindow>
#include <QInputDialog>
#include <QTreeWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QSplitter>
#include <QSettings>
#include <QMdiArea>
#include <QToolBar>
#include <QMenuBar>
#include <QPixmap>
#include <QMenu>
#include <QList>
#include <QIcon>

#include "collectioneditor.h"
#include "functionmanager.h"
#include "rgbmatrixeditor.h"
#include "functionwizard.h"
#include "chasereditor.h"
#include "scripteditor.h"
#include "sceneeditor.h"
#include "collection.h"
#include "efxeditor.h"
#include "rgbmatrix.h"
#include "function.h"
#include "apputil.h"
#include "chaser.h"
#include "script.h"
#include "scene.h"
#include "doc.h"
#include "efx.h"

#define PROP_ID Qt::UserRole
#define COL_NAME 0

#define SETTINGS_SPLITTER "functionmanager/splitter"

FunctionManager* FunctionManager::s_instance = NULL;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

FunctionManager::FunctionManager(QWidget* parent, Doc* doc, Qt::WindowFlags flags)
    : QWidget(parent, flags)
    , m_doc(doc)
{
    Q_ASSERT(doc != NULL);

    new QVBoxLayout(this);
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->setSpacing(0);

    initActions();
    initToolbar();
    initSplitterView();
    updateActionStatus();

    connect(m_doc, SIGNAL(modeChanged(Doc::Mode)),
            this, SLOT(slotModeChanged(Doc::Mode)));
    updateTree();

    m_tree->sortItems(COL_NAME, Qt::AscendingOrder);

    connect(m_doc, SIGNAL(clearing()), this, SLOT(slotDocClearing()));
    connect(m_doc, SIGNAL(functionChanged(quint32)), this, SLOT(slotFunctionChanged(quint32)));

    QSettings settings;
    QVariant var = settings.value(SETTINGS_SPLITTER);
    if (var.isValid() == true)
        m_splitter->restoreState(var.toByteArray());
    else
        m_splitter->setSizes(QList <int> () << int(this->width() / 2) << int(this->width() / 2));
}

FunctionManager::~FunctionManager()
{
    QSettings settings;
    settings.setValue(SETTINGS_SPLITTER, m_splitter->saveState());

    FunctionManager::s_instance = NULL;
}

FunctionManager* FunctionManager::instance()
{
    return s_instance;
}

void FunctionManager::createAndShow(QWidget* parent, Doc* doc)
{
    /* Must not create more than one instance */
    Q_ASSERT(s_instance == NULL);

    /* Create an MDI window for X11 & Win32 */
    QMdiArea* area = qobject_cast<QMdiArea*> (parent);
    Q_ASSERT(area != NULL);
    QMdiSubWindow* sub = new QMdiSubWindow;
    s_instance = new FunctionManager(sub, doc);
    sub->setWidget(s_instance);
    QWidget* window = area->addSubWindow(sub);

    connect(area, SIGNAL(subWindowActivated(QMdiSubWindow*)),
            s_instance, SLOT(slotSubWindowActivated(QMdiSubWindow*)));

    /* Set some common properties for the window and show it */
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setWindowIcon(QIcon(":/function.png"));
    window->setWindowTitle(tr("Functions"));
    window->setContextMenuPolicy(Qt::CustomContextMenu);

    sub->setSystemMenu(NULL);
}

void FunctionManager::slotModeChanged(Doc::Mode mode)
{
    /* Disable completely when in operate mode */
    if (mode == Doc::Operate)
    {
        m_toolbar->setEnabled(false);
        m_actionGroup->setEnabled(false);
    }
    else
    {
        m_toolbar->setEnabled(true);
        m_actionGroup->setEnabled(true);
    }
}

void FunctionManager::slotDocClearing()
{
    m_tree->clear();
    if (currentEditor() != NULL)
        delete currentEditor();
}

void FunctionManager::slotFunctionChanged(quint32 id)
{
    Function* function = m_doc->function(id);
    if (function == NULL)
        return;

    QTreeWidgetItem* item = functionItem(function);
    if (item != NULL)
        updateFunctionItem(item, function);
}

void FunctionManager::slotSubWindowActivated(QMdiSubWindow* sub)
{
    QMdiSubWindow* mySub = qobject_cast<QMdiSubWindow*> (parentWidget());
    Q_ASSERT(mySub != NULL);
    if (sub == mySub)
    {
        // FunctionManager has been activated
        emit functionManagerActive(true);
    }
    else if (sub != NULL)
    {
        // Another internal sub window has been activated
        emit functionManagerActive(false);
    }
    else
    {
        // Another application activated (not QLC)
    }
}

/*****************************************************************************
 * Menu, toolbar and actions
 *****************************************************************************/

void FunctionManager::initActions()
{
    m_actionGroup = new QActionGroup(this);

    /* Manage actions */
    m_addSceneAction = new QAction(QIcon(":/scene.png"),
                                   tr("New &scene"), this);
    m_addSceneAction->setShortcut(QKeySequence("CTRL+S"));
    m_actionGroup->addAction(m_addSceneAction);
    connect(m_addSceneAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddScene()));

    m_addChaserAction = new QAction(QIcon(":/chaser.png"),
                                    tr("New c&haser"), this);
    m_addChaserAction->setShortcut(QKeySequence("CTRL+H"));
    m_actionGroup->addAction(m_addChaserAction);
    connect(m_addChaserAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddChaser()));

    m_addCollectionAction = new QAction(QIcon(":/collection.png"),
                                        tr("New c&ollection"), this);
    m_addCollectionAction->setShortcut(QKeySequence("CTRL+O"));
    m_actionGroup->addAction(m_addCollectionAction);
    connect(m_addCollectionAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddCollection()));

    m_addEFXAction = new QAction(QIcon(":/efx.png"),
                                 tr("New E&FX"), this);
    m_addEFXAction->setShortcut(QKeySequence("CTRL+F"));
    m_actionGroup->addAction(m_addEFXAction);
    connect(m_addEFXAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddEFX()));

    m_addRGBMatrixAction = new QAction(QIcon(":/rgbmatrix.png"),
                                 tr("New &RGB Matrix"), this);
    m_addRGBMatrixAction->setShortcut(QKeySequence("CTRL+R"));
    m_actionGroup->addAction(m_addRGBMatrixAction);
    connect(m_addRGBMatrixAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddRGBMatrix()));

    m_addScriptAction = new QAction(QIcon(":/script.png"),
                                 tr("New scrip&t"), this);
    m_addScriptAction->setShortcut(QKeySequence("CTRL+T"));
    m_actionGroup->addAction(m_addScriptAction);
    connect(m_addScriptAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddScript()));

    m_wizardAction = new QAction(QIcon(":/wizard.png"),
                                 tr("Function Wizard"), this);
    m_wizardAction->setShortcut(QKeySequence("CTRL+A"));
    m_actionGroup->addAction(m_wizardAction);
    connect(m_wizardAction, SIGNAL(triggered(bool)),
            this, SLOT(slotWizard()));

    /* Edit actions */
    m_cloneAction = new QAction(QIcon(":/editcopy.png"),
                                tr("&Clone"), this);
    m_cloneAction->setShortcut(QKeySequence("CTRL+C"));
    m_actionGroup->addAction(m_cloneAction);
    connect(m_cloneAction, SIGNAL(triggered(bool)),
            this, SLOT(slotClone()));

    m_deleteAction = new QAction(QIcon(":/editdelete.png"),
                                 tr("&Delete"), this);
    m_deleteAction->setShortcut(QKeySequence("Delete"));
    m_actionGroup->addAction(m_deleteAction);
    connect(m_deleteAction, SIGNAL(triggered(bool)),
            this, SLOT(slotDelete()));

    m_selectAllAction = new QAction(QIcon(":/selectall.png"),
                                    tr("Select &all"), this);
    m_selectAllAction->setShortcut(QKeySequence("CTRL+A"));
    m_actionGroup->addAction(m_selectAllAction);
    connect(m_selectAllAction, SIGNAL(triggered(bool)),
            this, SLOT(slotSelectAll()));
}

void FunctionManager::initToolbar()
{
    // Add a toolbar to the dock area
    m_toolbar = new QToolBar("Function Manager", this);
    m_toolbar->setFloatable(false);
    m_toolbar->setMovable(false);
    layout()->addWidget(m_toolbar);
    m_toolbar->addAction(m_addSceneAction);
    m_toolbar->addAction(m_addChaserAction);
    m_toolbar->addAction(m_addEFXAction);
    m_toolbar->addAction(m_addCollectionAction);
    m_toolbar->addAction(m_addRGBMatrixAction);
    m_toolbar->addAction(m_addScriptAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_wizardAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_cloneAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_deleteAction);
}

void FunctionManager::slotAddScene()
{
    Function* f = new Scene(m_doc);
    if (m_doc->addFunction(f) == true)
    {
        addFunction(f);
    }
    else
    {
        QMessageBox::critical(this, tr("Function creation failed"),
                              tr("Unable to create new function."));
    }
}

void FunctionManager::slotAddChaser()
{
    Function* f = new Chaser(m_doc);
    if (m_doc->addFunction(f) == true)
    {
        addFunction(f);
    }
    else
    {
        QMessageBox::critical(this, tr("Function creation failed"),
                              tr("Unable to create new function."));
    }
}

void FunctionManager::slotAddCollection()
{
    Function* f = new Collection(m_doc);
    if (m_doc->addFunction(f) == true)
    {
        addFunction(f);
    }
    else
    {
        QMessageBox::critical(this, tr("Function creation failed"),
                              tr("Unable to create new function."));
    }
}

void FunctionManager::slotAddEFX()
{
    Function* f = new EFX(m_doc);
    if (m_doc->addFunction(f) == true)
    {
        addFunction(f);
    }
    else
    {
        QMessageBox::critical(this, tr("Function creation failed"),
                              tr("Unable to create new function."));
    }
}

void FunctionManager::slotAddRGBMatrix()
{
    Function* f = new RGBMatrix(m_doc);
    if (m_doc->addFunction(f) == true)
    {
        addFunction(f);
    }
    else
    {
        QMessageBox::critical(this, tr("Function creation failed"),
                              tr("Unable to create new function."));
    }
}

void FunctionManager::slotAddScript()
{
    Function* f = new Script(m_doc);
    if (m_doc->addFunction(f) == true)
    {
        addFunction(f);
    }
    else
    {
        QMessageBox::critical(this, tr("Function creation failed"),
                              tr("Unable to create new function."));
    }
}

void FunctionManager::slotWizard()
{
    FunctionWizard fw(this, m_doc);
    if (fw.exec() == QDialog::Accepted)
        updateTree();
}

void FunctionManager::slotClone()
{
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    while (it.hasNext() == true)
        copyFunction(itemFunctionId(it.next()));
}

void FunctionManager::slotDelete()
{
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    if (it.hasNext() == false)
        return;

    QString msg = tr("Do you want to DELETE functions:") + QString("\n");

    // Append functions' names to the message
    while (it.hasNext() == true)
        msg += it.next()->text(COL_NAME) + QString(", ");

    // Ask for user's confirmation
    if (QMessageBox::question(this, tr("Delete Functions"), msg,
                              QMessageBox::Yes, QMessageBox::No)
            == QMessageBox::Yes)
    {
        deleteSelectedFunctions();
        updateActionStatus();
        if (currentEditor() != NULL)
            delete currentEditor();
    }
}

void FunctionManager::slotSelectAll()
{
    /* This has to be done thru an intermediary slot because the tree
       widget hasn't been created when actions are being created and
       so a direct slot collection to m_tree is not possible. */
    m_tree->selectAll();
}

void FunctionManager::updateActionStatus()
{
    if (m_tree->selectedItems().isEmpty() == false)
    {
        /* At least one function has been selected, so
           editing is possible. */
        m_cloneAction->setEnabled(true);
        m_deleteAction->setEnabled(true);
        m_selectAllAction->setEnabled(true);
    }
    else
    {
        /* No functions selected */
        m_cloneAction->setEnabled(false);
        m_deleteAction->setEnabled(false);
        m_selectAllAction->setEnabled(false);
    }
}

/****************************************************************************
 * Function tree
 ****************************************************************************/

void FunctionManager::initSplitterView()
{
    m_splitter = new QSplitter(Qt::Horizontal, this);
    layout()->addWidget(m_splitter);
    initTree();
}

void FunctionManager::initTree()
{
    m_tree = new QTreeWidget(this);
    Q_ASSERT(m_splitter != NULL);
    m_splitter->addWidget(m_tree);

    // Add two columns for function and type
    QStringList labels;
    labels << tr("Function");
    m_tree->setHeaderLabels(labels);
    m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);
    m_tree->setRootIsDecorated(true);
    m_tree->setAllColumnsShowFocus(true);
    m_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tree->setSortingEnabled(true);
    m_tree->sortByColumn(COL_NAME, Qt::AscendingOrder);

    // Catch selection changes
    connect(m_tree, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotTreeSelectionChanged()));

    // Catch right-mouse clicks
    connect(m_tree, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(slotTreeContextMenuRequested()));
}

void FunctionManager::updateTree()
{
    m_tree->clear();
    foreach (Function* function, m_doc->functions())
        updateFunctionItem(new QTreeWidgetItem(parentItem(function)), function);
}

void FunctionManager::updateFunctionItem(QTreeWidgetItem* item, const Function* function)
{
    Q_ASSERT(item != NULL);
    Q_ASSERT(function != NULL);
    item->setText(COL_NAME, function->name());
    item->setIcon(COL_NAME, functionIcon(function));
    item->setData(COL_NAME, PROP_ID, function->id());
}

QTreeWidgetItem* FunctionManager::parentItem(const Function* function)
{
    Q_ASSERT(function != NULL);

    // Search for a parent item for function->type()
    for (int i = 0; i < m_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_tree->topLevelItem(i);
        Q_ASSERT(item != NULL);
        QVariant var = item->data(COL_NAME, Qt::UserRole);
        if (var.isValid() == false)
            continue;
        Function::Type type = (Function::Type) var.toInt();
        if (type == function->type())
            return item;
    }

    // Parent item for the given type doesn't exist yet so create one
    QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
    item->setText(COL_NAME, Function::typeToString(function->type()));
    item->setIcon(COL_NAME, functionIcon(function));
    item->setData(COL_NAME, Qt::UserRole, function->type());
    item->setFlags(Qt::ItemIsEnabled);
    return item;
}

quint32 FunctionManager::itemFunctionId(const QTreeWidgetItem* item) const
{
    if (item == NULL || item->parent() == NULL)
        return Function::invalidId();
    else
        return item->data(COL_NAME, PROP_ID).toUInt();
}

QTreeWidgetItem* FunctionManager::functionItem(const Function* function)
{
    Q_ASSERT(function != NULL);

    QTreeWidgetItem* parent = parentItem(function);
    Q_ASSERT(parent != NULL);

    for (int i = 0; i < parent->childCount(); i++)
    {
        QTreeWidgetItem* item = parent->child(i);
        if (itemFunctionId(item) == function->id())
            return item;
    }

    return NULL;
}

QIcon FunctionManager::functionIcon(const Function* function) const
{
    switch (function->type())
    {
    case Function::Scene:
        return QIcon(":/scene.png");
    case Function::Chaser:
        return QIcon(":/chaser.png");
    case Function::EFX:
        return QIcon(":/efx.png");
    case Function::Collection:
        return QIcon(":/collection.png");
    case Function::RGBMatrix:
        return QIcon(":/rgbmatrix.png");
    case Function::Script:
        return QIcon(":/script.png");
    default:
        return QIcon(":/function.png");
    }
}

void FunctionManager::deleteSelectedFunctions()
{
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item(it.next());
        quint32 fid = itemFunctionId(item);
        m_doc->deleteFunction(fid);

        QTreeWidgetItem* parent = item->parent();
        delete item;
        if (parent != NULL && parent->childCount() == 0)
            delete parent;
    }
}

void FunctionManager::slotTreeSelectionChanged()
{
    updateActionStatus();

    QList <QTreeWidgetItem*> selection(m_tree->selectedItems());
    if (selection.size() == 1)
    {
        Function* function = m_doc->function(itemFunctionId(selection.first()));
        if (function != NULL)
            editFunction(function);
    }
    else
    {
        if (currentEditor() != NULL)
            delete currentEditor();
    }
}

void FunctionManager::slotTreeContextMenuRequested()
{
    QMenu menu(this);
    menu.addAction(m_cloneAction);
    menu.addAction(m_selectAllAction);
    menu.addSeparator();
    menu.addAction(m_deleteAction);
    menu.addSeparator();
    menu.addAction(m_addSceneAction);
    menu.addAction(m_addChaserAction);
    menu.addAction(m_addEFXAction);
    menu.addAction(m_addCollectionAction);
    menu.addAction(m_addRGBMatrixAction);
    menu.addAction(m_addScriptAction);
    menu.addSeparator();
    menu.addAction(m_wizardAction);

    updateActionStatus();

    menu.exec(QCursor::pos());
}

/*****************************************************************************
 * Helpers
 *****************************************************************************/

void FunctionManager::copyFunction(quint32 fid)
{
    Function* function = m_doc->function(fid);
    Q_ASSERT(function != NULL);

    /* Attempt to create a copy of the function to Doc */
    Function* copy = function->createCopy(m_doc);
    if (copy != NULL)
    {
        copy->setName(tr("Copy of %1").arg(function->name()));

        /* Create a new item for the copied function */
        QTreeWidgetItem* parent = parentItem(function);
        QTreeWidgetItem* item = new QTreeWidgetItem(parent);
        updateFunctionItem(item, copy);
        parent->setExpanded(true);
    }
    else
    {
        QMessageBox::critical(this, tr("Function creation failed"),
                              tr("Unable to create new function."));
    }
}

void FunctionManager::addFunction(Function* function)
{
    Q_ASSERT(function != NULL);

    /* Create a new item for the function */
    QTreeWidgetItem* parent = parentItem(function);
    QTreeWidgetItem* item = new QTreeWidgetItem(parent);
    updateFunctionItem(item, function);
    parent->setExpanded(true);

    /* Clear current selection and select only the new one */
    m_tree->clearSelection();
    m_tree->sortItems(COL_NAME, Qt::AscendingOrder);
    m_tree->scrollToItem(item);
    m_tree->setCurrentItem(item);
    item->setSelected(true);

    /* Start editing immediately */
    editFunction(function);
}

void FunctionManager::editFunction(Function* function)
{
    // Destroy the existing editor, if it exists
    QWidget* editor = currentEditor();
    if (editor != NULL)
        delete editor;
    editor = NULL;
    Q_ASSERT(m_splitter->count() == 1);

    if (function == NULL)
        return;

    // Choose the editor by the selected function's type
    if (function->type() == Function::Scene)
    {
        editor = new SceneEditor(m_splitter, qobject_cast<Scene*> (function), m_doc);
        connect(this, SIGNAL(functionManagerActive(bool)),
                editor, SLOT(slotFunctionManagerActive(bool)));
    }
    else if (function->type() == Function::Chaser)
    {
        editor = new ChaserEditor(m_splitter, qobject_cast<Chaser*> (function), m_doc);
        connect(this, SIGNAL(functionManagerActive(bool)),
                editor, SLOT(slotFunctionManagerActive(bool)));
    }
    else if (function->type() == Function::Collection)
    {
        editor = new CollectionEditor(m_splitter, qobject_cast<Collection*> (function), m_doc);
    }
    else if (function->type() == Function::EFX)
    {
        editor = new EFXEditor(m_splitter, qobject_cast<EFX*> (function), m_doc);
        connect(this, SIGNAL(functionManagerActive(bool)),
                editor, SLOT(slotFunctionManagerActive(bool)));
    }
    else if (function->type() == Function::RGBMatrix)
    {
        editor = new RGBMatrixEditor(m_splitter, qobject_cast<RGBMatrix*> (function), m_doc);
        connect(this, SIGNAL(functionManagerActive(bool)),
                editor, SLOT(slotFunctionManagerActive(bool)));
    }
    else if (function->type() == Function::Script)
    {
        editor = new ScriptEditor(m_splitter, qobject_cast<Script*> (function), m_doc);
    }
    else
    {
        editor = NULL;
    }

    // Show the editor
    if (editor != NULL)
    {
        m_splitter->addWidget(editor);
        editor->show();
    }
}

QWidget* FunctionManager::currentEditor() const
{
    Q_ASSERT(m_splitter != NULL);
    if (m_splitter->count() < 2)
        return NULL;
    else
        return m_splitter->widget(1);
}
