/*
  Q Light Controller
  simpledesk.cpp

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

#include <QMdiSubWindow>
#include <QDomDocument>
#include <QDomElement>
#include <QMdiArea>
#include <QDebug>

#include "grandmasterslider.h"
#include "simpledeskengine.h"
#include "speeddialwidget.h"
#include "playbackslider.h"
#include "cuestackmodel.h"
#include "simpledesk.h"
#include "dmxslider.h"
#include "qlcmacros.h"
#include "cuestack.h"
#include "cue.h"
#include "doc.h"

#define PAGE_CHANNELS 12
#define PAGE_PLAYBACKS 12
#define PROP_ADDRESS "address"
#define PROP_PLAYBACK "playback"

#define COL_NUM  0
#define COL_NAME 1

SimpleDesk* SimpleDesk::s_instance = NULL;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

SimpleDesk::SimpleDesk(QWidget* parent, Doc* doc)
    : QWidget(parent)
    , m_engine(new SimpleDeskEngine(doc))
    , m_doc(doc)
    , m_selectedPlayback(UINT_MAX)
    , m_speedDials(NULL)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(doc != NULL);
    setupUi(this);

    initEngine();
    initUniverseSliders();
    initUniversePager();
    initGrandMaster();
    initPlaybackSliders();
    initCueStack();

    slotSelectPlayback(0);
}

SimpleDesk::~SimpleDesk()
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(m_engine != NULL);
    delete m_engine;
    m_engine = NULL;
}

SimpleDesk* SimpleDesk::instance()
{
    return s_instance;
}

void SimpleDesk::createAndShow(QWidget* parent, Doc* doc)
{
    qDebug() << Q_FUNC_INFO;
    /* Must not create more than one instance */
    Q_ASSERT(s_instance == NULL);

    /* Create an MDI window for X11 & Win32 */
    QMdiArea* area = qobject_cast<QMdiArea*> (parent);
    Q_ASSERT(area != NULL);
    QMdiSubWindow* sub = new QMdiSubWindow;
    s_instance = new SimpleDesk(sub, doc);
    sub->setWidget(s_instance);
    QWidget* window = area->addSubWindow(sub);

    /* Set some common properties for the window and show it */
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setWindowIcon(QIcon(":/slider.png"));
    window->setWindowTitle(tr("Simple Desk"));
    window->setContextMenuPolicy(Qt::CustomContextMenu);

    sub->setSystemMenu(NULL);
}

void SimpleDesk::clearContents()
{
    qDebug() << Q_FUNC_INFO;
    CueStackModel* model = qobject_cast<CueStackModel*> (m_cueStackView->model());
    Q_ASSERT(model != NULL);
    model->setCueStack(NULL);

    resetUniverseSliders();
    resetPlaybackSliders();
    m_engine->clearContents();
    slotSelectPlayback(0);
}

void SimpleDesk::initEngine()
{
    qDebug() << Q_FUNC_INFO;
    connect(m_engine, SIGNAL(cueStackStarted(uint)), this, SLOT(slotCueStackStarted(uint)));
    connect(m_engine, SIGNAL(cueStackStopped(uint)), this, SLOT(slotCueStackStopped(uint)));
}

/****************************************************************************
 * Universe controls
 ****************************************************************************/

void SimpleDesk::initUniverseSliders()
{
    qDebug() << Q_FUNC_INFO;
    new QHBoxLayout(m_universeGroup);
    for (int i = 0; i < PAGE_CHANNELS; i++)
    {
        DMXSlider* slider = new DMXSlider(m_universeGroup);
        m_universeGroup->layout()->addWidget(slider);
        m_universeSliders << slider;
        connect(slider, SIGNAL(valueChanged(uchar)), this, SLOT(slotUniverseSliderValueChanged(uchar)));
    }

    connect(m_doc, SIGNAL(fixtureAdded(quint32)), this, SLOT(slotUpdateUniverseSliders()));
    connect(m_doc, SIGNAL(fixtureRemoved(quint32)), this, SLOT(slotUpdateUniverseSliders()));
    connect(m_doc, SIGNAL(fixtureChanged(quint32)), this, SLOT(slotUpdateUniverseSliders()));
}

void SimpleDesk::initUniversePager()
{
    qDebug() << Q_FUNC_INFO;
    m_universePageSpin->setRange(1, int(512 / PAGE_CHANNELS) + 1);

    connect(m_universePageUpButton, SIGNAL(clicked()), this, SLOT(slotUniversePageUpClicked()));
    connect(m_universePageDownButton, SIGNAL(clicked()), this, SLOT(slotUniversePageDownClicked()));
    connect(m_universePageSpin, SIGNAL(valueChanged(int)), this, SLOT(slotUniversePageChanged(int)));
    connect(m_universeResetButton, SIGNAL(clicked()), this, SLOT(slotUniverseResetClicked()));

    slotUniversePageChanged(m_universePageSpin->minimum());
}

void SimpleDesk::resetUniverseSliders()
{
    qDebug() << Q_FUNC_INFO;
    QListIterator <DMXSlider*> it(m_universeSliders);
    while (it.hasNext() == true)
        it.next()->setValue(0);
}

void SimpleDesk::slotUniversePageUpClicked()
{
    qDebug() << Q_FUNC_INFO;
    m_universePageSpin->setValue(m_universePageSpin->value() + 1);
}

void SimpleDesk::slotUniversePageDownClicked()
{
    qDebug() << Q_FUNC_INFO;
    m_universePageSpin->setValue(m_universePageSpin->value() - 1);
}

void SimpleDesk::slotUniversePageChanged(int page)
{
    qDebug() << Q_FUNC_INFO;
    uint start = (page - 1) * PAGE_CHANNELS;
    for (int i = 0; i < PAGE_CHANNELS; i++)
    {
        DMXSlider* slider = m_universeSliders[i];
        Q_ASSERT(slider != NULL);
        if ((start + i) < 512)
        {
            slider->setEnabled(true);
            slider->setProperty(PROP_ADDRESS, start + i);
            slider->setLabel(QString::number(start + i + 1));

            disconnect(slider, SIGNAL(valueChanged(uchar)), this, SLOT(slotUniverseSliderValueChanged(uchar)));
            slider->setValue(m_engine->value(start + i));
            connect(slider, SIGNAL(valueChanged(uchar)), this, SLOT(slotUniverseSliderValueChanged(uchar)));

            Fixture* fxi = m_doc->fixture(m_doc->fixtureForAddress(start + i));
            if (fxi == NULL || fxi->isDimmer() == true)
            {
                slider->setVerticalLabel(tr("Intensity"));
                slider->setPalette(this->palette());
            }
            else
            {
                uint ch = (start + i) - fxi->universeAddress();
                const QLCChannel* channel = fxi->channel(ch);
                if (channel != NULL)
                {
                    slider->setVerticalLabel(channel->name());
                    if (channel->colour() != QLCChannel::NoColour)
                    {
                        QPalette pal(slider->palette());
                        pal.setColor(QPalette::WindowText, QColor(channel->colour()));
                        slider->setPalette(pal);
                    }
                    else
                    {
                        slider->setPalette(this->palette());
                    }
                }
                else
                {
                    slider->setVerticalLabel(tr("Intensity"));
                }
            }
        }
        else
        {
            slider->setEnabled(false);
            slider->setVerticalLabel(QString());
            slider->setProperty(PROP_ADDRESS, QVariant());
            slider->setValue(0);
            slider->setLabel("---");
            slider->setPalette(this->palette());
        }
    }
}

void SimpleDesk::slotUniverseResetClicked()
{
    qDebug() << Q_FUNC_INFO;
    resetUniverseSliders();
    m_engine->resetUniverse();
    slotUniversePageChanged(1);
}

void SimpleDesk::slotUniverseSliderValueChanged(uchar value)
{
    QVariant var(sender()->property(PROP_ADDRESS));
    if (var.isValid() == true) // Not true with disabled sliders
    {
        m_engine->setValue(var.toUInt(), value);

        if (m_editCueStackButton->isChecked() == true)
            replaceCurrentCue();
    }
}

void SimpleDesk::slotUpdateUniverseSliders()
{
    qDebug() << Q_FUNC_INFO;
    slotUniversePageChanged(m_universePageSpin->value());
}

/****************************************************************************
 * Grand Master
 ****************************************************************************/

void SimpleDesk::initGrandMaster()
{
    qDebug() << Q_FUNC_INFO;
    new QVBoxLayout(m_grandMasterContainer);
    m_grandMasterSlider = new GrandMasterSlider(m_grandMasterContainer, m_doc->outputMap(),
                                                m_doc->inputMap());
    m_grandMasterContainer->layout()->setMargin(0);
    m_grandMasterContainer->layout()->addWidget(m_grandMasterSlider);
    m_grandMasterSlider->refreshProperties();
}

/****************************************************************************
 * Playback Sliders
 ****************************************************************************/

void SimpleDesk::initPlaybackSliders()
{
    qDebug() << Q_FUNC_INFO;
    new QHBoxLayout(m_playbackGroup);
    for (int i = 0; i < PAGE_PLAYBACKS; i++)
    {
        PlaybackSlider* slider = new PlaybackSlider(m_playbackGroup);
        m_playbackGroup->layout()->addWidget(slider);
        slider->setLabel(QString::number(i + 1));
        slider->setProperty(PROP_PLAYBACK, uint(i));
        m_playbackSliders << slider;
        connect(slider, SIGNAL(selected()), this, SLOT(slotPlaybackSelected()));
        connect(slider, SIGNAL(started()), this, SLOT(slotPlaybackStarted()));
        connect(slider, SIGNAL(stopped()), this, SLOT(slotPlaybackStopped()));
        connect(slider, SIGNAL(flashing(bool)), this, SLOT(slotPlaybackFlashing(bool)));
        connect(slider, SIGNAL(valueChanged(uchar)), this, SLOT(slotPlaybackValueChanged(uchar)));
    }
}

void SimpleDesk::resetPlaybackSliders()
{
    qDebug() << Q_FUNC_INFO;
    QListIterator <PlaybackSlider*> it(m_playbackSliders);
    while (it.hasNext() == true)
        it.next()->setValue(0);
}

void SimpleDesk::slotPlaybackSelected()
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(sender() != NULL);
    uint pb = sender()->property(PROP_PLAYBACK).toUInt();
    if (m_selectedPlayback == pb)
        return;

    slotSelectPlayback(pb);
}

void SimpleDesk::slotSelectPlayback(uint pb)
{
    qDebug() << Q_FUNC_INFO;

    if (m_selectedPlayback != UINT_MAX)
        m_playbackSliders[m_selectedPlayback]->setSelected(false);

    if (pb != UINT_MAX)
        m_playbackSliders[pb]->setSelected(true);
    m_selectedPlayback = pb;

    CueStack* cueStack = m_engine->cueStack(pb);
    Q_ASSERT(cueStack != NULL);

    CueStackModel* model = qobject_cast<CueStackModel*> (m_cueStackView->model());
    Q_ASSERT(model != NULL);
    model->setCueStack(cueStack);

    updateCueStackButtons();
}

void SimpleDesk::slotPlaybackStarted()
{
    qDebug() << Q_FUNC_INFO;
    int pb = sender()->property(PROP_PLAYBACK).toUInt();
    CueStack* cueStack = m_engine->cueStack(pb);
    Q_ASSERT(cueStack != NULL);

    if (cueStack->isRunning() == false)
        cueStack->nextCue();
}

void SimpleDesk::slotPlaybackStopped()
{
    qDebug() << Q_FUNC_INFO;
    int pb = sender()->property(PROP_PLAYBACK).toUInt();
    CueStack* cueStack = m_engine->cueStack(pb);
    Q_ASSERT(cueStack != NULL);

    if (cueStack->isRunning() == true)
        cueStack->stop();
}

void SimpleDesk::slotPlaybackFlashing(bool enabled)
{
    int pb = sender()->property(PROP_PLAYBACK).toUInt();
    CueStack* cueStack = m_engine->cueStack(pb);
    Q_ASSERT(cueStack != NULL);

    cueStack->setFlashing(enabled);
}

void SimpleDesk::slotPlaybackValueChanged(uchar value)
{
    int pb = sender()->property(PROP_PLAYBACK).toUInt();
    CueStack* cueStack = m_engine->cueStack(pb);
    Q_ASSERT(cueStack != NULL);

    cueStack->adjustIntensity(qreal(value) / qreal(UCHAR_MAX));
}

/****************************************************************************
 * Cue Stack controls
 ****************************************************************************/

void SimpleDesk::initCueStack()
{
    qDebug() << Q_FUNC_INFO;
    CueStackModel* model = new CueStackModel(this);
    m_cueStackView->setModel(model);
    m_cueStackView->header()->setResizeMode(QHeaderView::ResizeToContents);

    connect(m_previousCueButton, SIGNAL(clicked()), this, SLOT(slotPreviousCueClicked()));
    connect(m_nextCueButton, SIGNAL(clicked()), this, SLOT(slotNextCueClicked()));
    connect(m_stopCueStackButton, SIGNAL(clicked()), this, SLOT(slotStopCueStackClicked()));
    connect(m_editCueStackButton, SIGNAL(clicked()), this, SLOT(slotEditCueStackClicked()));
    connect(m_recordCueButton, SIGNAL(clicked()), this, SLOT(slotRecordCueClicked()));

    connect(m_cueStackView->selectionModel(),
            SIGNAL(currentChanged(const QModelIndex&,const QModelIndex&)),
            this, SLOT(slotCueStackViewCurrentItemChanged(const QModelIndex&)));
}

void SimpleDesk::updateCueStackButtons()
{
    qDebug() << Q_FUNC_INFO;
    CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
    if (cueStack == NULL)
        return;

    m_stopCueStackButton->setEnabled(cueStack->isRunning());
    m_nextCueButton->setEnabled(cueStack->cues().size() > 0);
    m_previousCueButton->setEnabled(cueStack->cues().size() > 0);
}

void SimpleDesk::replaceCurrentCue()
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(m_selectedPlayback < uint(m_playbackSliders.size()));

    CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
    Q_ASSERT(cueStack != NULL);

    QModelIndex index = m_cueStackView->currentIndex();
    if (index.isValid() == true)
    {
        // Replace current cue values
        QString name = cueStack->cues().at(index.row()).name();
        Cue cue = m_engine->cue();
        cue.setName(name);
        cueStack->replaceCue(index.row(), cue);
    }
}

void SimpleDesk::updateSpeedDials()
{
    qDebug() << Q_FUNC_INFO;

    if (m_speedDials == NULL)
        return;

    CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
    Q_ASSERT(cueStack != NULL);

    QModelIndex index = m_cueStackView->currentIndex();
    if (index.row() >= 0 && index.row() < cueStack->cues().size())
    {
        Cue cue = cueStack->cues()[index.row()];
        m_speedDials->setWindowTitle(cue.name());
        m_speedDials->setFadeInSpeed(cue.fadeInSpeed());
        m_speedDials->setFadeOutSpeed(cue.fadeOutSpeed());
        m_speedDials->setDuration(cue.duration());

        m_speedDials->setOptionalTextTitle(tr("Cue name"));
        m_speedDials->setOptionalText(cue.name());
    }
    else
    {
        m_speedDials->setWindowTitle(tr("Cuestack %1").arg(m_selectedPlayback + 1));
    }
}

CueStack* SimpleDesk::currentCueStack() const
{
    Q_ASSERT(m_engine != NULL);
    CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
    Q_ASSERT(cueStack != NULL);
    return cueStack;
}

int SimpleDesk::currentCueIndex() const
{
    Q_ASSERT(m_cueStackView != NULL);
    return m_cueStackView->currentIndex().row();
}

void SimpleDesk::slotCueStackStarted(uint stack)
{
    qDebug() << Q_FUNC_INFO;
    if (stack != m_selectedPlayback)
        return;

    PlaybackSlider* slider = m_playbackSliders[m_selectedPlayback];
    Q_ASSERT(slider != NULL);
    if (slider->value() == 0)
        slider->setValue(UCHAR_MAX);
    updateCueStackButtons();
}

void SimpleDesk::slotCueStackStopped(uint stack)
{
    qDebug() << Q_FUNC_INFO;
    if (stack != m_selectedPlayback)
        return;

    PlaybackSlider* slider = m_playbackSliders[m_selectedPlayback];
    Q_ASSERT(slider != NULL);
    if (slider->value() != 0)
        slider->setValue(0);
    updateCueStackButtons();
}

void SimpleDesk::slotCueStackViewCurrentItemChanged(const QModelIndex& index)
{
    qDebug() << Q_FUNC_INFO;
    updateCueStackButtons();

    if (m_editCueStackButton->isChecked() == true)
    {
        CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
        Q_ASSERT(cueStack != NULL);

        if (index.row() >= 0 && index.row() < cueStack->cues().size())
        {
            Cue cue = cueStack->cues()[index.row()];
            m_engine->setCue(cue);
            slotUniversePageChanged(m_universePageSpin->value());
        }
    }

    updateSpeedDials();
}

void SimpleDesk::slotPreviousCueClicked()
{
    qDebug() << Q_FUNC_INFO;
    CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
    Q_ASSERT(cueStack != NULL);
    cueStack->previousCue();
}

void SimpleDesk::slotNextCueClicked()
{
    qDebug() << Q_FUNC_INFO;
    CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
    Q_ASSERT(cueStack != NULL);
    cueStack->nextCue();
}

void SimpleDesk::slotStopCueStackClicked()
{
    qDebug() << Q_FUNC_INFO;
    CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
    Q_ASSERT(cueStack != NULL);
    cueStack->stop();
}

void SimpleDesk::slotEditCueStackClicked()
{
    qDebug() << Q_FUNC_INFO;
    slotCueStackViewCurrentItemChanged(m_cueStackView->currentIndex());
    if (m_editCueStackButton->isChecked() == true)
    {
        if (m_speedDials == NULL)
        {
            m_speedDials = new SpeedDialWidget(this, Qt::Tool);
            m_speedDials->setAttribute(Qt::WA_DeleteOnClose);
            connect(m_speedDials, SIGNAL(fadeInChanged(uint)),
                    this, SLOT(slotFadeInDialChanged(uint)));
            connect(m_speedDials, SIGNAL(fadeOutChanged(uint)),
                    this, SLOT(slotFadeOutDialChanged(uint)));
            connect(m_speedDials, SIGNAL(durationChanged(uint)),
                    this, SLOT(slotDurationDialChanged(uint)));
            connect(m_speedDials, SIGNAL(optionalTextEdited(const QString&)),
                    this, SLOT(slotCueNameEdited(const QString&)));
        }

        m_speedDials->raise();
        m_speedDials->show();
        updateSpeedDials();
    }
    else
    {
        resetUniverseSliders();

        if (m_speedDials != NULL)
            delete m_speedDials;
        m_speedDials = NULL;
    }
}

void SimpleDesk::slotRecordCueClicked()
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(m_selectedPlayback < uint(m_playbackSliders.size()));

    CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
    Q_ASSERT(cueStack != NULL);

    QItemSelectionModel* model = m_cueStackView->selectionModel();
    Q_ASSERT(model != NULL);
    int index = 0;
    if (model->hasSelection() == true)
        index = model->currentIndex().row() + 1;
    else
        index = cueStack->cues().size();

    Cue cue = m_engine->cue();
    cue.setName(tr("Cue %1").arg(cueStack->cues().size() + 1));
    cueStack->insertCue(index, cue);

    // Select the newly-created Cue
    QItemSelection sel(model->model()->index(index, 0), model->model()->index(index, 1));
    model->select(sel, QItemSelectionModel::ClearAndSelect);
    model->setCurrentIndex(model->model()->index(index, 0), QItemSelectionModel::Current);

    updateCueStackButtons();
}

void SimpleDesk::slotFadeInDialChanged(uint ms)
{
    CueStack* cueStack = currentCueStack();
    int index = currentCueIndex();
    if (index >= 0 && index < cueStack->cues().size())
        cueStack->setFadeInSpeed(ms, index);
}

void SimpleDesk::slotFadeOutDialChanged(uint ms)
{
    CueStack* cueStack = currentCueStack();
    int index = currentCueIndex();
    if (index >= 0 && index < cueStack->cues().size())
        cueStack->setFadeOutSpeed(ms, index);
}

void SimpleDesk::slotDurationDialChanged(uint ms)
{
    CueStack* cueStack = currentCueStack();
    int index = currentCueIndex();
    if (index >= 0 && index < cueStack->cues().size())
        cueStack->setDuration(ms, index);
}

void SimpleDesk::slotCueNameEdited(const QString& name)
{
    CueStack* cueStack = currentCueStack();
    int index = currentCueIndex();
    if (index >= 0 && index < cueStack->cues().size())
        cueStack->setName(name, index);
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool SimpleDesk::loadXML(const QDomElement& root)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(m_engine != NULL);

    clearContents();

    if (root.tagName() != KXMLQLCSimpleDesk)
    {
        qWarning() << Q_FUNC_INFO << "Simple Desk node not found";
        return false;
    }

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCSimpleDeskEngine)
        {
            m_engine->loadXML(tag);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unrecognized Simple Desk node:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    slotSelectPlayback(0);

    return true;
}

bool SimpleDesk::saveXML(QDomDocument* doc, QDomElement* wksp_root) const
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);
    Q_ASSERT(m_engine != NULL);

    QDomElement root = doc->createElement(KXMLQLCSimpleDesk);
    wksp_root->appendChild(root);

    return m_engine->saveXML(doc, &root);
}

