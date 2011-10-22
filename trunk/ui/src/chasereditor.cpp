/*
  Q Light Controller
  chasereditor.cpp

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
#include <QLineEdit>
#include <QSettings>
#include <QDebug>

#include "qlcfixturedef.h"
#include "qlcmacros.h"

#include "functionselection.h"
#include "speedspinbox.h"
#include "chasereditor.h"
#include "mastertimer.h"
#include "chaserstep.h"
#include "outputmap.h"
#include "inputmap.h"
#include "apputil.h"
#include "fixture.h"
#include "chaser.h"
#include "doc.h"

#define SETTINGS_GEOMETRY "chasereditor/geometry"
#define PROP_STEP Qt::UserRole

#define KColumnNumber 0
#define KColumnName   1

ChaserEditor::ChaserEditor(QWidget* parent, Chaser* chaser, Doc* doc)
    : QDialog(parent)
    , m_doc(doc)
    , m_original(chaser)
{
    Q_ASSERT(chaser != NULL);
    Q_ASSERT(doc != NULL);

    setupUi(this);

    m_chaser = new Chaser(doc);
    Q_ASSERT(m_chaser != NULL);
    m_chaser->copyFrom(chaser);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    /* Resize columns to fit contents */
    m_list->header()->setResizeMode(QHeaderView::ResizeToContents);

    m_cutAction = new QAction(QIcon(":/editcut.png"), tr("Cut"), this);
    m_cutButton->setDefaultAction(m_cutAction);
    m_cutAction->setShortcut(QKeySequence(QKeySequence::Cut));
    connect(m_cutAction, SIGNAL(triggered(bool)), this, SLOT(slotCutClicked()));

    m_copyAction = new QAction(QIcon(":/editcopy.png"), tr("Copy"), this);
    m_copyButton->setDefaultAction(m_copyAction);
    m_copyAction->setShortcut(QKeySequence(QKeySequence::Copy));
    connect(m_copyAction, SIGNAL(triggered(bool)), this, SLOT(slotCopyClicked()));

    m_pasteAction = new QAction(QIcon(":/editpaste.png"), tr("Paste"), this);
    m_pasteButton->setDefaultAction(m_pasteAction);
    m_pasteAction->setShortcut(QKeySequence(QKeySequence::Paste));
    connect(m_pasteAction, SIGNAL(triggered(bool)), this, SLOT(slotPasteClicked()));

    /* Name edit */
    m_nameEdit->setText(m_chaser->name());
    m_nameEdit->setSelection(0, m_nameEdit->text().length());
    slotNameEdited(m_chaser->name());

    /* Speed */
    new QHBoxLayout(m_fadeInContainer);
    m_fadeInSpin = new SpeedSpinBox(SpeedSpinBox::Zero, m_fadeInContainer);
    m_fadeInContainer->layout()->addWidget(m_fadeInSpin);
    m_fadeInContainer->layout()->setMargin(0);
    m_fadeInSpin->setValue(m_chaser->fadeInSpeed());

    new QHBoxLayout(m_fadeOutContainer);
    m_fadeOutSpin = new SpeedSpinBox(SpeedSpinBox::Zero, m_fadeOutContainer);
    m_fadeOutContainer->layout()->addWidget(m_fadeOutSpin);
    m_fadeOutContainer->layout()->setMargin(0);
    m_fadeOutSpin->setValue(m_chaser->fadeOutSpeed());

    new QHBoxLayout(m_durationContainer);
    m_durationSpin = new SpeedSpinBox(SpeedSpinBox::Zero, m_durationContainer);
    m_durationContainer->layout()->addWidget(m_durationSpin);
    m_durationContainer->layout()->setMargin(0);
    m_durationSpin->setValue(m_chaser->duration());

    /* Running order */
    switch (m_chaser->runOrder())
    {
    default:
    case Chaser::Loop:
        m_loop->setChecked(true);
        break;
    case Chaser::PingPong:
        m_pingPong->setChecked(true);
        break;
    case Chaser::SingleShot:
        m_singleShot->setChecked(true);
        break;
    }

    /* Running direction */
    switch (m_chaser->direction())
    {
    default:
    case Chaser::Forward:
        m_forward->setChecked(true);
        break;
    case Chaser::Backward:
        m_backward->setChecked(true);
        break;
    }

    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameEdited(const QString&)));
    connect(m_add, SIGNAL(clicked()),
            this, SLOT(slotAddClicked()));
    connect(m_remove, SIGNAL(clicked()),
            this, SLOT(slotRemoveClicked()));
    connect(m_raise, SIGNAL(clicked()),
            this, SLOT(slotRaiseClicked()));
    connect(m_lower, SIGNAL(clicked()),
            this, SLOT(slotLowerClicked()));

    connect(m_fadeInSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotFadeInSpinChanged(int)));
    connect(m_fadeOutSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotFadeOutSpinChanged(int)));
    connect(m_durationSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotDurationSpinChanged(int)));

    connect(m_loop, SIGNAL(clicked()),
            this, SLOT(slotLoopClicked()));
    connect(m_singleShot, SIGNAL(clicked()),
            this, SLOT(slotSingleShotClicked()));
    connect(m_pingPong, SIGNAL(clicked()),
            this, SLOT(slotPingPongClicked()));

    connect(m_forward, SIGNAL(clicked()),
            this, SLOT(slotForwardClicked()));
    connect(m_backward, SIGNAL(clicked()),
            this, SLOT(slotBackwardClicked()));

    connect(m_list, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotItemSelectionChanged()));

    /* Chaser steps */
    QListIterator <ChaserStep> it(m_chaser->steps());
    while (it.hasNext() == true)
        updateItem(new QTreeWidgetItem(m_list), it.next());

    /* Window position */
    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());
    AppUtil::ensureWidgetIsVisible(this);

    updateClipboardButtons();
}

ChaserEditor::~ChaserEditor()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());

    delete m_chaser;
    m_chaser = NULL;
}

void ChaserEditor::accept()
{
    m_original->copyFrom(m_chaser);
    QDialog::accept();
}

void ChaserEditor::slotNameEdited(const QString& text)
{
    m_chaser->setName(text);
    setWindowTitle(QString(tr("Chaser editor - %1")).arg(text));
}

/****************************************************************************
 * List manipulation
 ****************************************************************************/

void ChaserEditor::slotAddClicked()
{
    FunctionSelection fs(this, m_doc);
    fs.setDisabledFunctions(QList <quint32>() << m_chaser->id());

    if (fs.exec() == QDialog::Accepted)
    {
        int insertionPoint = m_list->topLevelItemCount();
        QTreeWidgetItem* item = m_list->currentItem();
        if (item != NULL)
            insertionPoint = m_list->indexOfTopLevelItem(item);

#warning insert AFTER, not before

        /* Append selected functions */
        QListIterator <quint32> it(fs.selection());
        while (it.hasNext() == true)
        {
            ChaserStep step(it.next());
            QTreeWidgetItem* item = new QTreeWidgetItem;
            updateItem(item, step);
            m_list->insertTopLevelItem(insertionPoint++, item);
        }

        m_list->setCurrentItem(item);
        updateStepNumbers();

        updateClipboardButtons();
    }
}

void ChaserEditor::slotRemoveClicked()
{
    slotCutClicked();
    m_clipboard.clear();
    updateClipboardButtons();
}

void ChaserEditor::slotRaiseClicked()
{
    QList <QTreeWidgetItem*> items(m_list->selectedItems());
    QListIterator <QTreeWidgetItem*> it(items);

    // Check, whether even one of the items would "bleed" over the edge and
    // cancel the operation if that is the case.
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item(it.next());
        int index = m_list->indexOfTopLevelItem(item);
        if (index == 0)
            return;
    }

    // Move the items
    it.toFront();
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item(it.next());
        int index = m_list->indexOfTopLevelItem(item);
        m_list->takeTopLevelItem(index);
        m_list->insertTopLevelItem(index - 1, item);
    }

    updateStepNumbers();
    updateChaserContents();

    // Select the moved items
    it.toFront();
    while (it.hasNext() == true)
        it.next()->setSelected(true);

    updateClipboardButtons();
}

void ChaserEditor::slotLowerClicked()
{
    QList <QTreeWidgetItem*> items(m_list->selectedItems());
    QListIterator <QTreeWidgetItem*> it(items);

    // Check, whether even one of the items would "bleed" over the edge and
    // cancel the operation if that is the case.
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item(it.next());
        int index = m_list->indexOfTopLevelItem(item);
        if (index == m_list->topLevelItemCount() - 1)
            return;
    }

    // Move the items
    it.toBack();
    while (it.hasPrevious() == true)
    {
        QTreeWidgetItem* item(it.previous());
        int index = m_list->indexOfTopLevelItem(item);
        m_list->takeTopLevelItem(index);
        m_list->insertTopLevelItem(index + 1, item);
    }

    updateStepNumbers();
    updateChaserContents();

    // Select the items
    it.toFront();
    while (it.hasNext() == true)
        it.next()->setSelected(true);

    updateClipboardButtons();
}

void ChaserEditor::slotItemSelectionChanged()
{
    updateClipboardButtons();
}

/****************************************************************************
 * Clipboard
 ****************************************************************************/

void ChaserEditor::slotCutClicked()
{
    m_clipboard.clear();
    QListIterator <QTreeWidgetItem*> it(m_list->selectedItems());
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item(it.next());
        m_clipboard << stepAtItem(item);
        delete item;
    }

    m_list->setCurrentItem(NULL);

    updateStepNumbers();
    updateChaserContents();
    updateClipboardButtons();
}

void ChaserEditor::slotCopyClicked()
{
    m_clipboard.clear();
    QListIterator <QTreeWidgetItem*> it(m_list->selectedItems());
    while (it.hasNext() == true)
        m_clipboard << stepAtItem(it.next());
    updateClipboardButtons();
}

void ChaserEditor::slotPasteClicked()
{
    if (m_clipboard.isEmpty() == true)
        return;

#warning insert AFTER not before

    int insertionPoint = 0;
    QTreeWidgetItem* currentItem = m_list->currentItem();
    if (currentItem != NULL)
    {
        insertionPoint = m_list->indexOfTopLevelItem(currentItem);
        currentItem->setSelected(false);
    }
    else
    {
        insertionPoint = CLAMP(0, 0, m_list->topLevelItemCount() - 1);
    }

    QListIterator <ChaserStep> it(m_clipboard);
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem;
        updateItem(item, it.next());
        m_list->insertTopLevelItem(insertionPoint, item);
        item->setSelected(true);
        insertionPoint = CLAMP(m_list->indexOfTopLevelItem(item) + 1, 0, m_list->topLevelItemCount() - 1);
    }

    updateStepNumbers();
    updateChaserContents();
    updateClipboardButtons();
}

/****************************************************************************
 * Run order & Direction
 ****************************************************************************/

void ChaserEditor::slotLoopClicked()
{
    m_chaser->setRunOrder(Function::Loop);
}

void ChaserEditor::slotSingleShotClicked()
{
    m_chaser->setRunOrder(Function::SingleShot);
}

void ChaserEditor::slotPingPongClicked()
{
    m_chaser->setRunOrder(Function::PingPong);
}

void ChaserEditor::slotForwardClicked()
{
    m_chaser->setDirection(Function::Forward);
}

void ChaserEditor::slotBackwardClicked()
{
    m_chaser->setDirection(Function::Backward);
}

/****************************************************************************
 * Speed
 ****************************************************************************/

void ChaserEditor::slotFadeInSpinChanged(int ms)
{
    m_chaser->setFadeInSpeed(ms);
}

void ChaserEditor::slotFadeOutSpinChanged(int ms)
{
    m_chaser->setFadeOutSpeed(ms);
}

void ChaserEditor::slotDurationSpinChanged(int ms)
{
    m_chaser->setDuration(ms);
}

/****************************************************************************
 * Utilities
 ****************************************************************************/

void ChaserEditor::updateItem(QTreeWidgetItem* item, const ChaserStep& step)
{
    Function* function = step.resolveFunction(m_doc);
    Q_ASSERT(function != NULL);
    Q_ASSERT(item != NULL);

    item->setText(KColumnNumber, QString("%1").arg(m_list->indexOfTopLevelItem(item) + 1));
    item->setText(KColumnName, function->name());
    item->setData(0, PROP_STEP, step.toVariant());
}

void ChaserEditor::updateStepNumbers()
{
    for (int i = 0; i < m_list->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_list->topLevelItem(i);
        Q_ASSERT(item != NULL);
        item->setText(KColumnNumber, QString("%1").arg(i + 1));
    }
}

ChaserStep ChaserEditor::stepAtItem(const QTreeWidgetItem* item) const
{
    Q_ASSERT(item != NULL);
    QVariant var = item->data(0, PROP_STEP);
    if (var.isValid() == true)
        return ChaserStep::fromVariant(var);
    else
        return ChaserStep();
}

ChaserStep ChaserEditor::stepAtIndex(int index) const
{
    if (index < 0 || index >= m_list->topLevelItemCount())
        return ChaserStep();

    QTreeWidgetItem* item = m_list->topLevelItem(index);
    return stepAtItem(item);
}

void ChaserEditor::updateChaserContents()
{
    Q_ASSERT(m_chaser != NULL);

    m_chaser->clear();
    for (int i = 0; i < m_list->topLevelItemCount(); i++)
    {
        ChaserStep step = stepAtIndex(i);
        m_chaser->addStep(step);
    }
}

void ChaserEditor::updateClipboardButtons()
{
    if (m_list->selectedItems().size() > 0)
    {
        m_cutAction->setEnabled(true);
        m_copyAction->setEnabled(true);
    }
    else
    {
        m_cutAction->setEnabled(false);
        m_copyAction->setEnabled(false);
    }

    if (m_clipboard.size() > 0)
        m_pasteAction->setEnabled(true);
    else
        m_pasteAction->setEnabled(false);
}
