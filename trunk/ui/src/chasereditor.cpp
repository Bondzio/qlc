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
#include "speeddialwidget.h"
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

#define COL_NUM      0
#define COL_NAME     1
#define COL_FADEIN   2
#define COL_FADEOUT  3
#define COL_DURATION 4

ChaserEditor::ChaserEditor(QWidget* parent, Chaser* chaser, Doc* doc)
    : QWidget(parent)
    , m_doc(doc)
    , m_chaser(chaser)
{
    Q_ASSERT(chaser != NULL);
    Q_ASSERT(doc != NULL);

    setupUi(this);

    /* Resize columns to fit contents */
    m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);

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

    /* Speed */
    new QHBoxLayout(m_fadeInContainer);
    m_fadeInSpin = new SpeedSpinBox(SpeedSpinBox::Zero, m_fadeInContainer);
    m_fadeInContainer->layout()->addWidget(m_fadeInSpin);
    m_fadeInContainer->layout()->setMargin(0);
    m_fadeInSpin->setValue(m_chaser->fadeInSpeed());
    m_fadeInCheck->setChecked(m_chaser->isGlobalFadeIn());
    m_fadeInSpin->setEnabled(m_chaser->isGlobalFadeIn());

    new QHBoxLayout(m_fadeOutContainer);
    m_fadeOutSpin = new SpeedSpinBox(SpeedSpinBox::Zero, m_fadeOutContainer);
    m_fadeOutContainer->layout()->addWidget(m_fadeOutSpin);
    m_fadeOutContainer->layout()->setMargin(0);
    m_fadeOutSpin->setValue(m_chaser->fadeOutSpeed());
    m_fadeOutCheck->setChecked(m_chaser->isGlobalFadeOut());
    m_fadeOutSpin->setEnabled(m_chaser->isGlobalFadeOut());

    new QHBoxLayout(m_durationContainer);
    m_durationSpin = new SpeedSpinBox(SpeedSpinBox::Zero, m_durationContainer);
    m_durationContainer->layout()->addWidget(m_durationSpin);
    m_durationContainer->layout()->setMargin(0);
    m_durationSpin->setValue(m_chaser->duration());
    m_durationCheck->setChecked(m_chaser->isGlobalDuration());
    m_durationSpin->setEnabled(m_chaser->isGlobalDuration());

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
    connect(m_fadeInCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotFadeInChecked(bool)));
    connect(m_fadeOutCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotFadeOutChecked(bool)));
    connect(m_durationCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotDurationChecked(bool)));

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

    connect(m_tree, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotItemSelectionChanged()));

    updateTree(true);
    updateClipboardButtons();

    // Set focus to the editor
    m_nameEdit->setFocus();
}

ChaserEditor::~ChaserEditor()
{
}

void ChaserEditor::slotFunctionManagerActive(bool active)
{
    if (active == true)
    {
        updateSpeedDials();
    }
    else
    {
        if (m_speedDials != NULL)
            delete m_speedDials;
        m_speedDials = NULL;
    }
}

void ChaserEditor::slotNameEdited(const QString& text)
{
    m_chaser->setName(text);
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
        int insertionPoint = m_tree->topLevelItemCount();
        QTreeWidgetItem* item = m_tree->currentItem();
        if (item != NULL)
            insertionPoint = m_tree->indexOfTopLevelItem(item) + 1;

        /* Append selected functions */
        QListIterator <quint32> it(fs.selection());
        while (it.hasNext() == true)
        {
            ChaserStep step(it.next());
            QTreeWidgetItem* item = new QTreeWidgetItem;
            updateItem(item, step);
            m_tree->insertTopLevelItem(insertionPoint++, item);
        }

        m_tree->setCurrentItem(item);
        updateStepNumbers();
        updateChaserContents();

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
    QList <QTreeWidgetItem*> items(m_tree->selectedItems());
    QListIterator <QTreeWidgetItem*> it(items);

    // Check, whether even one of the items would "bleed" over the edge and
    // cancel the operation if that is the case.
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item(it.next());
        int index = m_tree->indexOfTopLevelItem(item);
        if (index == 0)
            return;
    }

    // Move the items
    it.toFront();
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item(it.next());
        int index = m_tree->indexOfTopLevelItem(item);
        m_tree->takeTopLevelItem(index);
        m_tree->insertTopLevelItem(index - 1, item);
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
    QList <QTreeWidgetItem*> items(m_tree->selectedItems());
    QListIterator <QTreeWidgetItem*> it(items);

    // Check, whether even one of the items would "bleed" over the edge and
    // cancel the operation if that is the case.
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item(it.next());
        int index = m_tree->indexOfTopLevelItem(item);
        if (index == m_tree->topLevelItemCount() - 1)
            return;
    }

    // Move the items
    it.toBack();
    while (it.hasPrevious() == true)
    {
        QTreeWidgetItem* item(it.previous());
        int index = m_tree->indexOfTopLevelItem(item);
        m_tree->takeTopLevelItem(index);
        m_tree->insertTopLevelItem(index + 1, item);
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
    updateSpeedDials();
}

/****************************************************************************
 * Clipboard
 ****************************************************************************/

void ChaserEditor::slotCutClicked()
{
    m_clipboard.clear();
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item(it.next());
        m_clipboard << stepAtItem(item);
        delete item;
    }

    m_tree->setCurrentItem(NULL);

    updateStepNumbers();
    updateChaserContents();
    updateClipboardButtons();
}

void ChaserEditor::slotCopyClicked()
{
    m_clipboard.clear();
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    while (it.hasNext() == true)
        m_clipboard << stepAtItem(it.next());
    updateClipboardButtons();
}

void ChaserEditor::slotPasteClicked()
{
    if (m_clipboard.isEmpty() == true)
        return;

    int insertionPoint = 0;
    QTreeWidgetItem* currentItem = m_tree->currentItem();
    if (currentItem != NULL)
    {
        insertionPoint = m_tree->indexOfTopLevelItem(currentItem) + 1;
        currentItem->setSelected(false);
    }
    else
    {
        insertionPoint = m_tree->topLevelItemCount();
    }

    QListIterator <ChaserStep> it(m_clipboard);
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem;
        updateItem(item, it.next());
        m_tree->insertTopLevelItem(insertionPoint, item);
        item->setSelected(true);
        insertionPoint = CLAMP(m_tree->indexOfTopLevelItem(item) + 1, 0, m_tree->topLevelItemCount() - 1);
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

void ChaserEditor::slotFadeInChecked(bool state)
{
    m_fadeInSpin->setEnabled(state);
    m_chaser->setGlobalFadeIn(state);
    updateTree();
    updateSpeedDials();
}

void ChaserEditor::slotFadeOutChecked(bool state)
{
    m_fadeOutSpin->setEnabled(state);
    m_chaser->setGlobalFadeOut(state);
    updateTree();
    updateSpeedDials();
}

void ChaserEditor::slotDurationChecked(bool state)
{
    m_durationSpin->setEnabled(state);
    m_chaser->setGlobalDuration(state);
    updateTree();
    updateSpeedDials();
}

void ChaserEditor::slotFadeInDialChanged(uint ms)
{
    foreach (QTreeWidgetItem* item, m_tree->selectedItems())
    {
        int index = m_tree->indexOfTopLevelItem(item);
        ChaserStep step = stepAtItem(item);
        step.fadeIn = ms;
        m_chaser->replaceStep(step, index);
        updateItem(item, step);
    }
}

void ChaserEditor::slotFadeOutDialChanged(uint ms)
{
    foreach (QTreeWidgetItem* item, m_tree->selectedItems())
    {
        int index = m_tree->indexOfTopLevelItem(item);
        ChaserStep step = stepAtItem(item);
        step.fadeOut = ms;
        m_chaser->replaceStep(step, index);
        updateItem(item, step);
    }
}

void ChaserEditor::slotDurationDialChanged(uint ms)
{
    foreach (QTreeWidgetItem* item, m_tree->selectedItems())
    {
        int index = m_tree->indexOfTopLevelItem(item);
        ChaserStep step = stepAtItem(item);
        step.duration = ms;
        m_chaser->replaceStep(step, index);
        updateItem(item, step);
    }
}

void ChaserEditor::updateSpeedDials()
{
    QList <QTreeWidgetItem*> selected(m_tree->selectedItems());
    if (selected.size() == 0 ||
        (m_fadeInCheck->isChecked() == true &&
         m_fadeOutCheck->isChecked() == true &&
         m_durationCheck->isChecked() == true))
    {
        // Nothing is selected or all speeds are global
        if (m_speedDials != NULL)
            delete m_speedDials;
        m_speedDials = NULL;
    }
    else
    {
        if (m_speedDials == NULL)
        {
            m_speedDials = new SpeedDialWidget(this);
            m_speedDials->setAttribute(Qt::WA_DeleteOnClose);

            connect(m_speedDials, SIGNAL(fadeInChanged(uint)),
                    this, SLOT(slotFadeInDialChanged(uint)));
            connect(m_speedDials, SIGNAL(fadeOutChanged(uint)),
                    this, SLOT(slotFadeOutDialChanged(uint)));
            connect(m_speedDials, SIGNAL(durationChanged(uint)),
                    this, SLOT(slotDurationDialChanged(uint)));

            m_speedDials->show();
        }

        if (m_speedDials == NULL)
            return;

        if (selected.size() == 1)
        {
            const QTreeWidgetItem* item(selected.first());
            QString title = QString("%1: %2").arg(item->text(COL_NUM))
                                             .arg(item->text(COL_NAME));
            m_speedDials->setWindowTitle(title);

            const ChaserStep step(stepAtItem(item));
            m_speedDials->setFadeInSpeed(step.fadeIn);
            m_speedDials->setFadeOutSpeed(step.fadeOut);
            m_speedDials->setDuration(step.duration);
        }
        else
        {
            m_speedDials->setWindowTitle(tr("Multiple Steps"));
            m_speedDials->setFadeInSpeed(0);
            m_speedDials->setFadeOutSpeed(0);
            m_speedDials->setDuration(0);
        }

        m_speedDials->setFadeInEnabled(!m_fadeInCheck->isChecked());
        m_speedDials->setFadeOutEnabled(!m_fadeOutCheck->isChecked());
        m_speedDials->setDurationEnabled(!m_durationCheck->isChecked());
    }
}

/****************************************************************************
 * Utilities
 ****************************************************************************/

void ChaserEditor::updateTree(bool clear)
{
    if (clear == true)
        m_tree->clear();

    for (int i = 0; i < m_chaser->steps().size(); i++)
    {
        QTreeWidgetItem* item = NULL;

        if (clear == true)
            item = new QTreeWidgetItem(m_tree);
        else
            item = m_tree->topLevelItem(i);
        Q_ASSERT(item != NULL);

        updateItem(item, m_chaser->steps().at(i));
    }
}

void ChaserEditor::updateItem(QTreeWidgetItem* item, const ChaserStep& step)
{
    Function* function = step.resolveFunction(m_doc);
    Q_ASSERT(function != NULL);
    Q_ASSERT(item != NULL);

    item->setData(COL_NUM, PROP_STEP, step.toVariant());
    item->setText(COL_NUM, QString("%1").arg(m_tree->indexOfTopLevelItem(item) + 1));
    item->setText(COL_NAME, function->name());

    if (m_fadeInCheck->isChecked() == false)
        item->setText(COL_FADEIN, AppUtil::speedText(step.fadeIn));
    else
        item->setText(COL_FADEIN, QString());

    if (m_fadeOutCheck->isChecked() == false)
        item->setText(COL_FADEOUT, AppUtil::speedText(step.fadeOut));
    else
        item->setText(COL_FADEOUT, QString());

    if (m_durationCheck->isChecked() == false)
        item->setText(COL_DURATION, AppUtil::speedText(step.duration));
    else
        item->setText(COL_DURATION, QString());
}

void ChaserEditor::updateStepNumbers()
{
    for (int i = 0; i < m_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_tree->topLevelItem(i);
        Q_ASSERT(item != NULL);
        item->setText(COL_NUM, QString("%1").arg(i + 1));
    }
}

ChaserStep ChaserEditor::stepAtItem(const QTreeWidgetItem* item) const
{
    Q_ASSERT(item != NULL);
    QVariant var = item->data(COL_NUM, PROP_STEP);
    if (var.isValid() == true)
        return ChaserStep::fromVariant(var);
    else
        return ChaserStep();
}

ChaserStep ChaserEditor::stepAtIndex(int index) const
{
    if (index < 0 || index >= m_tree->topLevelItemCount())
        return ChaserStep();

    QTreeWidgetItem* item = m_tree->topLevelItem(index);
    return stepAtItem(item);
}

void ChaserEditor::updateChaserContents()
{
    Q_ASSERT(m_chaser != NULL);

    m_chaser->clear();
    for (int i = 0; i < m_tree->topLevelItemCount(); i++)
    {
        ChaserStep step = stepAtIndex(i);
        m_chaser->addStep(step);
    }
}

void ChaserEditor::updateClipboardButtons()
{
    if (m_tree->selectedItems().size() > 0)
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
