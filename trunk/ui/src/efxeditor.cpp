/*
  Q Light Controller
  efxeditor.cpp

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
#include <QMessageBox>
#include <QPaintEvent>
#include <QSettings>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QPainter>
#include <QLabel>
#include <QDebug>
#include <QPen>

#include "qlcfixturedef.h"
#include "qlcchannel.h"

#include "fixtureselection.h"
#include "efxpreviewarea.h"
#include "vcdockslider.h"
#include "efxeditor.h"
#include "fixture.h"
#include "apputil.h"
#include "doc.h"
#include "app.h"

#define SETTINGS_GEOMETRY "efxeditor/geometry"

#define KColumnNumber  0
#define KColumnName    1
#define KColumnReverse 2
#define KColumnIntensity 3

#define PROPERTY_FIXTURE "fixture"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

EFXEditor::EFXEditor(QWidget* parent, EFX* efx, Doc* doc)
    : QDialog(parent)
    , m_doc(doc)
    , m_original(efx)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(efx != NULL);

    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    /* Create a copy of the original scene so that we can freely modify it.
       Keep also a pointer to the original so that we can move the
       contents from the copied chaser to the original when OK is clicked */
    m_efx = new EFX(doc);
    m_efx->copyFrom(efx);
    Q_ASSERT(m_efx != NULL);

    initGeneralPage();
    initMovementPage();

    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());
    AppUtil::ensureWidgetIsVisible(this);

    // Used for intensity changes
    m_testTimer.setSingleShot(true);
    m_testTimer.setInterval(500);
    connect(&m_testTimer, SIGNAL(timeout()), this, SLOT(slotRestartTest()));
}

EFXEditor::~EFXEditor()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());

    if (m_efx->stopped() == false)
        m_efx->stopAndWait();

    delete m_efx;
}

void EFXEditor::initGeneralPage()
{
    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameEdited(const QString&)));

    connect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotFixtureItemChanged(QTreeWidgetItem*,int)));

    connect(m_addFixtureButton, SIGNAL(clicked()),
            this, SLOT(slotAddFixtureClicked()));
    connect(m_removeFixtureButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveFixtureClicked()));

    connect(m_raiseFixtureButton, SIGNAL(clicked()),
            this, SLOT(slotRaiseFixtureClicked()));
    connect(m_lowerFixtureButton, SIGNAL(clicked()),
            this, SLOT(slotLowerFixtureClicked()));

    connect(m_parallelRadio, SIGNAL(toggled(bool)),
            this, SLOT(slotParallelRadioToggled(bool)));
    connect(m_serialRadio, SIGNAL(toggled(bool)),
            this, SLOT(slotSerialRadioToggled(bool)));
    connect(m_asymmetricRadio, SIGNAL(toggled(bool)),
            this, SLOT(slotAsymmetricRadioToggled(bool)));

    connect(m_fadeInSpin, SIGNAL(valueChanged(double)),
            this, SLOT(slotFadeInSpinChanged(double)));
    connect(m_fadeOutSpin, SIGNAL(valueChanged(double)),
            this, SLOT(slotFadeOutSpinChanged(double)));
    connect(m_patternSpin, SIGNAL(valueChanged(double)),
            this, SLOT(slotPatternSpinChanged(double)));

    // Test slots
    connect(m_testButton, SIGNAL(clicked()),
            this, SLOT(slotTestClicked()));
    connect(m_raiseFixtureButton, SIGNAL(clicked()),
            this, SLOT(slotRestartTest()));
    connect(m_lowerFixtureButton, SIGNAL(clicked()),
            this, SLOT(slotRestartTest()));
    connect(m_parallelRadio, SIGNAL(toggled(bool)),
            this, SLOT(slotRestartTest()));
    connect(m_serialRadio, SIGNAL(toggled(bool)),
            this, SLOT(slotRestartTest()));
    connect(m_asymmetricRadio, SIGNAL(toggled(bool)),
            this, SLOT(slotRestartTest()));
    connect(m_fadeInSpin, SIGNAL(valueChanged(double)),
            this, SLOT(slotRestartTest()));

    m_fadeInSpin->setValue(m_efx->fadeIn());
    m_fadeOutSpin->setValue(m_efx->fadeOut());
    m_patternSpin->setValue(m_efx->patternSpeed());

    /* Set the EFX's name to the name field */
    m_nameEdit->setText(m_efx->name());
    slotNameEdited(m_efx->name());

    /* Resize columns to fit contents */
    m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);

    /* Put all of the EFX's fixtures to the tree view */
    QListIterator <EFXFixture*> it(m_efx->fixtures());
    while (it.hasNext() == true)
        addFixtureItem(it.next());

    /* Set propagation mode */
    if (m_efx->propagationMode() == EFX::Serial)
        m_serialRadio->setChecked(true);
    else if (m_efx->propagationMode() == EFX::Asymmetric)
        m_asymmetricRadio->setChecked(true);
    else
        m_parallelRadio->setChecked(true);
}

void EFXEditor::initMovementPage()
{
    new QHBoxLayout(m_previewFrame);
    m_previewArea = new EFXPreviewArea(m_previewFrame);
    m_previewFrame->layout()->setMargin(0);
    m_previewFrame->layout()->addWidget(m_previewArea);

    /* Get supported algorithms and fill the algorithm combo with them */
    m_algorithmCombo->addItems(EFX::algorithmList());

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

    connect(m_algorithmCombo, SIGNAL(activated(const QString&)),
            this, SLOT(slotAlgorithmSelected(const QString&)));
    connect(m_widthSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotWidthSpinChanged(int)));
    connect(m_heightSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotHeightSpinChanged(int)));
    connect(m_xOffsetSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotXOffsetSpinChanged(int)));
    connect(m_yOffsetSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotYOffsetSpinChanged(int)));
    connect(m_rotationSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotRotationSpinChanged(int)));

    connect(m_xFrequencySpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotXFrequencySpinChanged(int)));
    connect(m_yFrequencySpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotYFrequencySpinChanged(int)));
    connect(m_xPhaseSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotXPhaseSpinChanged(int)));
    connect(m_yPhaseSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotYPhaseSpinChanged(int)));

    QString algo(EFX::algorithmToString(m_efx->algorithm()));
    /* Select the EFX's algorithm from the algorithm combo */
    for (int i = 0; i < m_algorithmCombo->count(); i++)
    {
        if (m_algorithmCombo->itemText(i) == algo)
        {
            m_algorithmCombo->setCurrentIndex(i);
            break;
        }
    }

    /* Causes the EFX function to update the preview point array */
    slotAlgorithmSelected(algo);

    /* Get the algorithm parameters */
    m_widthSpin->setValue(m_efx->width());
    m_heightSpin->setValue(m_efx->height());
    m_xOffsetSpin->setValue(m_efx->xOffset());
    m_yOffsetSpin->setValue(m_efx->yOffset());
    m_rotationSpin->setValue(m_efx->rotation());

    m_xFrequencySpin->setValue(m_efx->xFrequency());
    m_yFrequencySpin->setValue(m_efx->yFrequency());
    m_xPhaseSpin->setValue(m_efx->xPhase());
    m_yPhaseSpin->setValue(m_efx->yPhase());

    /* Running order */
    switch (m_efx->runOrder())
    {
    default:
    case Function::Loop:
        m_loop->setChecked(true);
        break;
    case Function::SingleShot:
        m_singleShot->setChecked(true);
        break;
    case Function::PingPong:
        m_pingPong->setChecked(true);
        break;
    }

    /* Direction */
    switch (m_efx->direction())
    {
    default:
    case Function::Forward:
        m_forward->setChecked(true);
        break;
    case Function::Backward:
        m_backward->setChecked(true);
        break;
    }

    redrawPreview();
}

void EFXEditor::accept()
{
    m_efx->setName(m_nameEdit->text());

    /* Copy the contents of the modified EFX over the original EFX */
    m_original->copyFrom(m_efx);

    QDialog::accept();
}

void EFXEditor::slotTestClicked()
{
    if (m_testButton->isChecked() == true)
        m_doc->masterTimer()->startFunction(m_efx, false);
    else
        m_efx->stopAndWait();
}

void EFXEditor::slotRestartTest()
{
    if (m_testButton->isChecked() == true)
    {
        // Toggle off, toggle on. Duh.
        m_testButton->click();
        m_testButton->click();
    }
}

/*****************************************************************************
 * General page
 *****************************************************************************/

QTreeWidgetItem* EFXEditor::fixtureItem(EFXFixture* ef)
{
    QTreeWidgetItemIterator it(m_tree);
    while (*it != NULL)
    {
        QTreeWidgetItem* item = *it;
        EFXFixture* ef_item = reinterpret_cast<EFXFixture*>
                              (item->data(0, Qt::UserRole).toULongLong());
        if (ef_item == ef)
            return item;
        ++it;
    }

    return NULL;
}

const QList <EFXFixture*> EFXEditor::selectedFixtures() const
{
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    QList <EFXFixture*> list;

    /* Put all selected fixture IDs to a list and return it */
    while (it.hasNext() == true)
    {
        EFXFixture* ef = reinterpret_cast <EFXFixture*>
                         (it.next()->data(0, Qt::UserRole).toULongLong());
        list << ef;
    }

    return list;
}

void EFXEditor::updateIndices(int from, int to)
{
    QTreeWidgetItem* item;
    int i;

    for (i = from; i <= to; i++)
    {
        item = m_tree->topLevelItem(i);
        Q_ASSERT(item != NULL);

        item->setText(KColumnNumber,
                      QString("%1").arg(i + 1, 3, 10, QChar('0')));
    }
}

void EFXEditor::addFixtureItem(EFXFixture* ef)
{
    QTreeWidgetItem* item;
    Fixture* fxi;

    Q_ASSERT(ef != NULL);

    fxi = m_doc->fixture(ef->fixture());
    if (fxi == NULL)
        return;

    item = new QTreeWidgetItem(m_tree);
    item->setText(KColumnName, fxi->name());
    item->setData(0, Qt::UserRole, QVariant(reinterpret_cast<qulonglong> (ef)));
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);

    if (ef->direction() == Function::Backward)
        item->setCheckState(KColumnReverse, Qt::Checked);
    else
        item->setCheckState(KColumnReverse, Qt::Unchecked);

    updateIntensityColumn(item, ef);

    updateIndices(m_tree->indexOfTopLevelItem(item),
                  m_tree->topLevelItemCount() - 1);

    /* Select newly-added fixtures so that they can be moved quickly */
    m_tree->setCurrentItem(item);
}

void EFXEditor::updateIntensityColumn(QTreeWidgetItem* item, EFXFixture* ef)
{
    Q_ASSERT(item != NULL);
    Q_ASSERT(ef != NULL);

    if (m_tree->itemWidget(item, KColumnIntensity) == NULL)
    {
        QSpinBox* spin = new QSpinBox(m_tree);
        spin->setAutoFillBackground(true);
        spin->setRange(0, 255);
        spin->setValue(ef->fadeIntensity());
        m_tree->setItemWidget(item, KColumnIntensity, spin);
        spin->setProperty(PROPERTY_FIXTURE, (qulonglong) ef);
        connect(spin, SIGNAL(valueChanged(int)),
                this, SLOT(slotFixtureIntensityChanged(int)));
    }
}

void EFXEditor::removeFixtureItem(EFXFixture* ef)
{
    QTreeWidgetItem* item;
    int from;

    Q_ASSERT(ef != NULL);

    item = fixtureItem(ef);
    Q_ASSERT(item != NULL);

    from = m_tree->indexOfTopLevelItem(item);
    delete m_tree->itemWidget(item, KColumnIntensity);
    delete item;

    updateIndices(from, m_tree->topLevelItemCount() - 1);
}

void EFXEditor::slotNameEdited(const QString &text)
{
    setWindowTitle(tr("EFX - %1").arg(text));
}

void EFXEditor::slotFixtureItemChanged(QTreeWidgetItem* item, int column)
{
    if (column == KColumnReverse)
    {
        EFXFixture* ef = reinterpret_cast <EFXFixture*>
                         (item->data(0, Qt::UserRole).toULongLong());
        Q_ASSERT(ef != NULL);

        if (item->checkState(column) == Qt::Checked)
            ef->setDirection(Function::Backward);
        else
            ef->setDirection(Function::Forward);
    }
}

void EFXEditor::slotFixtureIntensityChanged(int intensity)
{
    QSpinBox* spin = qobject_cast<QSpinBox*>(QObject::sender());
    Q_ASSERT(spin != NULL);
    EFXFixture* ef = (EFXFixture*) spin->property(PROPERTY_FIXTURE).toULongLong();
    Q_ASSERT(ef != NULL);
    ef->setFadeIntensity(uchar(intensity));

    // Restart the test after the latest intensity change, delayed
    m_testTimer.start();
}

void EFXEditor::slotAddFixtureClicked()
{
    /* Put all fixtures already present into a list of fixtures that
       will be disabled in the fixture selection dialog */
    QList <quint32> disabled;
    QTreeWidgetItemIterator twit(m_tree);
    while (*twit != NULL)
    {
        EFXFixture* ef = reinterpret_cast <EFXFixture*>
                         ((*twit)->data(0, Qt::UserRole).toULongLong());
        Q_ASSERT(ef != NULL);

        disabled.append(ef->fixture());
        twit++;
    }

    /* Disable all fixtures that don't have pan OR tilt channels */
    QListIterator <Fixture*> fxit(m_doc->fixtures());
    while (fxit.hasNext() == true)
    {
        Fixture* fixture(fxit.next());
        Q_ASSERT(fixture != NULL);

        // If a channel with pan group exists, don't disable this fixture
        if (fixture->channel("", Qt::CaseSensitive, QLCChannel::Pan)
                != QLCChannel::invalid())
        {
            continue;
        }

        // If a channel with tilt group exists, don't disable this fixture
        if (fixture->channel("", Qt::CaseSensitive, QLCChannel::Tilt)
                != QLCChannel::invalid())
        {
            continue;
        }

        // Disable all fixtures without pan or tilt channels
        disabled << fixture->id();
    }

    /* Get a list of new fixtures to add to the scene */
    FixtureSelection fs(this, m_doc, true, disabled);
    if (fs.exec() == QDialog::Accepted)
    {
        // Stop test while adding fixtures
        bool testing = false;
        if (m_testButton->isChecked() == true)
        {
            m_testButton->click();
            testing = true;
        }

        QListIterator <quint32> it(fs.selection);
        while (it.hasNext() == true)
        {
            EFXFixture* ef = new EFXFixture(m_efx);
            ef->setFixture(it.next());

            if (m_efx->addFixture(ef) == true)
                addFixtureItem(ef);
            else
                delete ef;
        }

        // Restart test if appropriate
        if (testing == true)
            m_testButton->click();
    }
}

void EFXEditor::slotRemoveFixtureClicked()
{
    int r = QMessageBox::question(
                this, tr("Remove fixtures"),
                tr("Do you want to remove the selected fixture(s)?"),
                QMessageBox::Yes, QMessageBox::No);

    bool testing = false;
    if (m_testButton->isChecked() == true)
    {
        m_testButton->click();
        testing = true;
    }

    if (r == QMessageBox::Yes)
    {
        QListIterator <EFXFixture*> it(selectedFixtures());
        while (it.hasNext() == true)
        {
            EFXFixture* ef = it.next();
            Q_ASSERT(ef != NULL);

            removeFixtureItem(ef);
            if (m_efx->removeFixture(ef) == true)
                delete ef;
        }
    }

    if (testing == true)
        m_testButton->click();
}

void EFXEditor::slotRaiseFixtureClicked()
{
    QTreeWidgetItem* item = m_tree->currentItem();
    if (item != NULL)
    {
        int index = m_tree->indexOfTopLevelItem(item);
        if (index == 0)
            return;

        EFXFixture* ef = reinterpret_cast <EFXFixture*>
                         (item->data(0, Qt::UserRole).toULongLong());
        Q_ASSERT(ef != NULL);

        if (m_efx->raiseFixture(ef) == true)
        {
            item = m_tree->takeTopLevelItem(index);

            m_tree->insertTopLevelItem(index - 1, item);
            m_tree->setCurrentItem(item);
            updateIntensityColumn(item, ef);

            updateIndices(index - 1, index);
        }
    }
}

void EFXEditor::slotLowerFixtureClicked()
{
    QTreeWidgetItem* item = m_tree->currentItem();
    if (item != NULL)
    {
        int index = m_tree->indexOfTopLevelItem(item);
        if (index == (m_tree->topLevelItemCount() - 1))
            return;

        EFXFixture* ef = reinterpret_cast <EFXFixture*>
                         (item->data(0, Qt::UserRole).toULongLong());
        Q_ASSERT(ef != NULL);

        if (m_efx->lowerFixture(ef) == true)
        {
            item = m_tree->takeTopLevelItem(index);
            m_tree->insertTopLevelItem(index + 1, item);
            m_tree->setCurrentItem(item);
            updateIntensityColumn(item, ef);

            updateIndices(index, index + 1);
        }
    }
}

void EFXEditor::slotParallelRadioToggled(bool state)
{
    Q_ASSERT(m_efx != NULL);
    if (state == true)
        m_efx->setPropagationMode(EFX::Parallel);
}

void EFXEditor::slotSerialRadioToggled(bool state)
{
    Q_ASSERT(m_efx != NULL);
    if (state == true)
        m_efx->setPropagationMode(EFX::Serial);
}

void EFXEditor::slotAsymmetricRadioToggled(bool state)
{
    Q_ASSERT(m_efx != NULL);
    if (state == true)
        m_efx->setPropagationMode(EFX::Asymmetric);
}

void EFXEditor::slotFadeInSpinChanged(double seconds)
{
    m_efx->setFadeIn(seconds);
}

void EFXEditor::slotFadeOutSpinChanged(double seconds)
{
    m_efx->setFadeOut(seconds);
}

void EFXEditor::slotPatternSpinChanged(double seconds)
{
    m_efx->setPatternSpeed(seconds);
}

/*****************************************************************************
 * Movement page
 *****************************************************************************/

void EFXEditor::slotAlgorithmSelected(const QString &text)
{
    Q_ASSERT(m_efx != NULL);

    EFX::Algorithm algo = EFX::stringToAlgorithm(text);
    m_efx->setAlgorithm(algo);

    if (m_efx->isFrequencyEnabled())
    {
        m_xFrequencyLabel->setEnabled(true);
        m_yFrequencyLabel->setEnabled(true);

        m_xFrequencySpin->setEnabled(true);
        m_yFrequencySpin->setEnabled(true);
    }
    else
    {
        m_xFrequencyLabel->setEnabled(false);
        m_yFrequencyLabel->setEnabled(false);

        m_xFrequencySpin->setEnabled(false);
        m_yFrequencySpin->setEnabled(false);
    }

    if (m_efx->isPhaseEnabled())
    {
        m_xPhaseLabel->setEnabled(true);
        m_yPhaseLabel->setEnabled(true);

        m_xPhaseSpin->setEnabled(true);
        m_yPhaseSpin->setEnabled(true);
    }
    else
    {
        m_xPhaseLabel->setEnabled(false);
        m_yPhaseLabel->setEnabled(false);

        m_xPhaseSpin->setEnabled(false);
        m_yPhaseSpin->setEnabled(false);
    }

    redrawPreview();
}

void EFXEditor::slotWidthSpinChanged(int value)
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setWidth(value);
    redrawPreview();
}

void EFXEditor::slotHeightSpinChanged(int value)
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setHeight(value);
    redrawPreview();
}

void EFXEditor::slotRotationSpinChanged(int value)
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setRotation(value);
    redrawPreview();
}

void EFXEditor::slotXOffsetSpinChanged(int value)
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setXOffset(value);
    redrawPreview();
}

void EFXEditor::slotYOffsetSpinChanged(int value)
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setYOffset(value);
    redrawPreview();
}

void EFXEditor::slotXFrequencySpinChanged(int value)
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setXFrequency(value);
    redrawPreview();
}

void EFXEditor::slotYFrequencySpinChanged(int value)
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setYFrequency(value);
    redrawPreview();
}

void EFXEditor::slotXPhaseSpinChanged(int value)
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setXPhase(value);
    redrawPreview();
}

void EFXEditor::slotYPhaseSpinChanged(int value)
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setYPhase(value);
    redrawPreview();
}

/*****************************************************************************
 * Run order
 *****************************************************************************/

void EFXEditor::slotLoopClicked()
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setRunOrder(Function::Loop);
}

void EFXEditor::slotSingleShotClicked()
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setRunOrder(Function::SingleShot);
}

void EFXEditor::slotPingPongClicked()
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setRunOrder(Function::PingPong);
}

/*****************************************************************************
 * Direction
 *****************************************************************************/

void EFXEditor::slotForwardClicked()
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setDirection(Function::Forward);

    m_previewArea->setReverse(false);
    m_previewArea->draw();
}

void EFXEditor::slotBackwardClicked()
{
    Q_ASSERT(m_efx != NULL);
    m_efx->setDirection(Function::Backward);

    m_previewArea->setReverse(true);
    m_previewArea->draw();
}

void EFXEditor::redrawPreview()
{
    QVector <QPoint> points;
    m_efx->preview(points);
    m_previewArea->setPoints(points);
    m_previewArea->draw();
}
