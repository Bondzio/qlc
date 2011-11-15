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
#include <QMdiArea>

#include "grandmasterslider.h"
#include "simpledeskengine.h"
#include "playbackslider.h"
#include "simpledesk.h"
#include "dmxslider.h"
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
    , m_selectedPlayback(-1)
{
    Q_ASSERT(doc != NULL);
    setupUi(this);

    initUniverseSliders();
    initUniversePager();
    initGrandMaster();
    initPlaybackSliders();
    initCueStack();
}

SimpleDesk::~SimpleDesk()
{
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
        slider->setProperty(PROP_PLAYBACK, i);
        m_playbackSliders << slider;
        connect(slider, SIGNAL(selected()), this, SLOT(slotPlaybackSelected()));
        connect(slider, SIGNAL(valueChanged(uchar)), this, SLOT(slotPlaybackValueChanged(uchar)));
    }

    slotSelectPlayback(0);
}

void SimpleDesk::slotPlaybackSelected()
{
    int pb = sender()->property(PROP_PLAYBACK).toInt();
    if (m_selectedPlayback == pb)
        return;

    slotSelectPlayback(pb);
}

void SimpleDesk::slotSelectPlayback(int pb)
{
    if (m_selectedPlayback != -1)
        m_playbackSliders[m_selectedPlayback]->setSelected(false);

    if (pb != -1)
        m_playbackSliders[pb]->setSelected(true);
    m_selectedPlayback = pb;

    m_cueList->clear();

    CueStack* cueStack = m_engine->cueStack(pb);
    Q_ASSERT(cueStack != NULL);
    foreach (const Cue& cue, cueStack->cues())
        updateCueItem(new QTreeWidgetItem(m_cueList), cue);
}

void SimpleDesk::slotPlaybackValueChanged(uchar value)
{
    PlaybackSlider* slider = qobject_cast<PlaybackSlider*> (sender());
    Q_ASSERT(slider != NULL);
}

/****************************************************************************
 * Cue Stack controls
 ****************************************************************************/

void SimpleDesk::initCueStack()
{
    connect(m_previousCueButton, SIGNAL(clicked()), this, SLOT(slotPreviousCueClicked()));
    connect(m_nextCueButton, SIGNAL(clicked()), this, SLOT(slotNextCueClicked()));
    connect(m_stopCueStackButton, SIGNAL(clicked()), this, SLOT(slotStopCueStackClicked()));
    connect(m_configureCueStackButton, SIGNAL(clicked()), this, SLOT(slotConfigureCueStackClicked()));
    connect(m_storeCueButton, SIGNAL(clicked()), this, SLOT(slotStoreCueClicked()));
    connect(m_recordCueButton, SIGNAL(clicked()), this, SLOT(slotRecordCueClicked()));
}

void SimpleDesk::updateCueItem(QTreeWidgetItem* item, const Cue& cue)
{
    Q_ASSERT(item != NULL);
    int index = m_cueList->indexOfTopLevelItem(item);
    item->setText(COL_NUM, QString::number(index + 1));
    item->setText(COL_NAME, cue.name());
}

void SimpleDesk::slotPreviousCueClicked()
{
}

void SimpleDesk::slotNextCueClicked()
{
}

void SimpleDesk::slotStopCueStackClicked()
{
}

void SimpleDesk::slotConfigureCueStackClicked()
{
}

void SimpleDesk::slotStoreCueClicked()
{
}

void SimpleDesk::slotRecordCueClicked()
{
    Q_ASSERT(m_selectedPlayback != -1);

    Cue cue;
    QHashIterator <uint,uchar> it(m_engine->values());
    while (it.hasNext() == true)
    {
        it.next();
        cue.setValue(it.key(), it.value());
    }

    CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
    Q_ASSERT(cueStack != NULL);
    cue.setName(tr("Cue %1").arg(m_cueList->topLevelItemCount() + 1));
    cueStack->addCue(cue);
    updateCueItem(new QTreeWidgetItem(m_cueList), cue);
}
