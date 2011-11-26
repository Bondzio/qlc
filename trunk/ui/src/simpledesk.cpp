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
{
    Q_ASSERT(doc != NULL);
    setupUi(this);

    initEngine();
    initUniverseSliders();
    initUniversePager();
    initGrandMaster();
    initPlaybackSliders();
    initCueStack();

//    slotSelectPlayback(0);
}

SimpleDesk::~SimpleDesk()
{
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
    CueStackModel* model = qobject_cast<CueStackModel*> (m_cueStackView->model());
    Q_ASSERT(model != NULL);
    model->setCueStack(NULL);

    resetUniverseSliders();
    resetPlaybackSliders();
    m_engine->clearContents();
    //slotSelectPlayback(0);
}

void SimpleDesk::initEngine()
{
    connect(m_engine, SIGNAL(cueStackStarted(uint)), this, SLOT(slotCueStackStarted(uint)));
    connect(m_engine, SIGNAL(cueStackStopped(uint)), this, SLOT(slotCueStackStopped(uint)));
}

/****************************************************************************
 * Universe controls
 ****************************************************************************/

void SimpleDesk::initUniverseSliders()
{
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
    m_universePageSpin->setRange(1, int(512 / PAGE_CHANNELS) + 1);

    connect(m_universePageUpButton, SIGNAL(clicked()), this, SLOT(slotUniversePageUpClicked()));
    connect(m_universePageDownButton, SIGNAL(clicked()), this, SLOT(slotUniversePageDownClicked()));
    connect(m_universePageSpin, SIGNAL(valueChanged(int)), this, SLOT(slotUniversePageChanged(int)));

    slotUniversePageChanged(m_universePageSpin->minimum());
}

void SimpleDesk::resetUniverseSliders()
{
    QListIterator <DMXSlider*> it(m_universeSliders);
    while (it.hasNext() == true)
        it.next()->setValue(0);
}

void SimpleDesk::slotUniversePageUpClicked()
{
    m_universePageSpin->setValue(m_universePageSpin->value() + 1);
}

void SimpleDesk::slotUniversePageDownClicked()
{
    m_universePageSpin->setValue(m_universePageSpin->value() - 1);
}

void SimpleDesk::slotUniversePageChanged(int page)
{
    uint start = (page - 1) * PAGE_CHANNELS;
    for (int i = 0; i < PAGE_CHANNELS; i++)
    {
        DMXSlider* slider = m_universeSliders[i];
        Q_ASSERT(slider != NULL);
        if ((start + i) < 512)
        {
            slider->setEnabled(true);
            slider->setProperty(PROP_ADDRESS, start + i);
            slider->setValue(m_engine->value(start + i));
            slider->setLabel(QString::number(start + i + 1));

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

void SimpleDesk::slotUniverseSliderValueChanged(uchar value)
{
    QVariant var(sender()->property(PROP_ADDRESS));
    if (var.isValid() == true) // Not true with disabled sliders
        m_engine->setValue(var.toUInt(), value);
}

void SimpleDesk::slotUpdateUniverseSliders()
{
    slotUniversePageChanged(m_universePageSpin->value());
}

/****************************************************************************
 * Grand Master
 ****************************************************************************/

void SimpleDesk::initGrandMaster()
{
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
        connect(slider, SIGNAL(valueChanged(uchar)), this, SLOT(slotPlaybackValueChanged(uchar)));
    }
}

void SimpleDesk::resetPlaybackSliders()
{
    QListIterator <PlaybackSlider*> it(m_playbackSliders);
    while (it.hasNext() == true)
        it.next()->setValue(0);
}

void SimpleDesk::slotPlaybackSelected()
{
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
    int pb = sender()->property(PROP_PLAYBACK).toUInt();
    CueStack* cueStack = m_engine->cueStack(pb);
    Q_ASSERT(cueStack != NULL);

    if (cueStack->isRunning() == false)
        cueStack->nextCue();
}

void SimpleDesk::slotPlaybackStopped()
{
    int pb = sender()->property(PROP_PLAYBACK).toUInt();
    CueStack* cueStack = m_engine->cueStack(pb);
    Q_ASSERT(cueStack != NULL);

    if (cueStack->isRunning() == true)
        cueStack->stop();
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
    CueStackModel* model = new CueStackModel(this);
    m_cueStackView->setModel(model);

    connect(m_previousCueButton, SIGNAL(clicked()), this, SLOT(slotPreviousCueClicked()));
    connect(m_nextCueButton, SIGNAL(clicked()), this, SLOT(slotNextCueClicked()));
    connect(m_stopCueStackButton, SIGNAL(clicked()), this, SLOT(slotStopCueStackClicked()));
    connect(m_configureCueStackButton, SIGNAL(clicked()), this, SLOT(slotConfigureCueStackClicked()));
    connect(m_editCueStackButton, SIGNAL(clicked()), this, SLOT(slotEditCueStackClicked()));
    connect(m_storeCueButton, SIGNAL(clicked()), this, SLOT(slotStoreCueClicked()));
    connect(m_recordCueButton, SIGNAL(clicked()), this, SLOT(slotRecordCueClicked()));
}

void SimpleDesk::updateCueStackButtons()
{
    CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
    if (cueStack == NULL)
        return;

    m_stopCueStackButton->setEnabled(cueStack->isRunning());
    m_nextCueButton->setEnabled(cueStack->cues().size() > 0);
    m_previousCueButton->setEnabled(cueStack->cues().size() > 0);

    // QTreeWidgetItem* item = m_cueList->currentItem();
    // m_storeCueButton->setEnabled(item != NULL);
}

void SimpleDesk::slotCueStackStarted(uint stack)
{
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
    if (stack != m_selectedPlayback)
        return;

    PlaybackSlider* slider = m_playbackSliders[m_selectedPlayback];
    Q_ASSERT(slider != NULL);
    if (slider->value() != 0)
        slider->setValue(0);
    updateCueStackButtons();
}

void SimpleDesk::slotCueStackCurrentItemChanged()
{
    updateCueStackButtons();

    if (m_editCueStackButton->isChecked() == true)
    {
        CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
        Q_ASSERT(cueStack != NULL);
/*
        int index = m_cueList->indexOfTopLevelItem(m_cueList->currentItem());
        if (index >= 0 && index < cueStack->cues().size())
        {
            Cue cue = cueStack->cues()[index];
            m_engine->setCue(cue);
            slotUniversePageChanged(m_universePageSpin->value());
        }
*/
    }
}

void SimpleDesk::slotPreviousCueClicked()
{
    CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
    Q_ASSERT(cueStack != NULL);
    cueStack->previousCue();
}

void SimpleDesk::slotNextCueClicked()
{
    CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
    Q_ASSERT(cueStack != NULL);
    cueStack->nextCue();
}

void SimpleDesk::slotStopCueStackClicked()
{
    CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
    Q_ASSERT(cueStack != NULL);
    cueStack->stop();
}

void SimpleDesk::slotConfigureCueStackClicked()
{
}

void SimpleDesk::slotEditCueStackClicked()
{
    slotCueStackCurrentItemChanged();
}

void SimpleDesk::slotStoreCueClicked()
{
    Q_ASSERT(m_selectedPlayback < uint(m_playbackSliders.size()));

    CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
    Q_ASSERT(cueStack != NULL);

    int index = m_cueStackView->selectionModel()->currentIndex().row();

    Cue cue = m_engine->cue();
    if (index >= cueStack->cues().size())
    {
        cue.setName(tr("Cue %1").arg(cueStack->cues().size() + 1));
        cueStack->appendCue(cue);
    }
    else
    {
        Cue old = cueStack->cues()[index];
        cue.setName(old.name());
        cueStack->replaceCue(index, cue);
    }
}

void SimpleDesk::slotRecordCueClicked()
{
    Q_ASSERT(m_selectedPlayback < uint(m_playbackSliders.size()));

    int index = m_cueStackView->selectionModel()->currentIndex().row() + 1;

    CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
    Q_ASSERT(cueStack != NULL);

    Cue cue = m_engine->cue();
    cue.setName(tr("Cue %1").arg(cueStack->cues().size() + 1));
    cueStack->insertCue(index, cue);
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool SimpleDesk::loadXML(const QDomElement& root)
{
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
    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);
    Q_ASSERT(m_engine != NULL);

    QDomElement root = doc->createElement(KXMLQLCSimpleDesk);
    wksp_root->appendChild(root);

    return m_engine->saveXML(doc, &root);
}

