/*
  Q Light Controller
  cuestack.cpp

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

#include <QDebug>
#include <QHash>

#include "universearray.h"
#include "genericfader.h"
#include "fadechannel.h"
#include "mastertimer.h"
#include "qlcmacros.h"
#include "cuestack.h"
#include "cue.h"
#include "doc.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

CueStack::CueStack(Doc* doc)
    : QObject(doc)
    , m_fadeInSpeed(500)
    , m_fadeOutSpeed(500)
    , m_duration(1000)
    , m_currentIndex(-1)
    , m_running(false)
    , m_intensity(1.0)
    , m_fader(NULL)
    , m_elapsed(0)
    , m_previous(false)
    , m_next(false)
{
    Q_ASSERT(doc != NULL);
}

CueStack::~CueStack()
{
    if (m_fader != NULL)
        delete m_fader;
    m_fader = NULL;
}

Doc* CueStack::doc() const
{
    return qobject_cast<Doc*> (parent());
}

/****************************************************************************
 * Speed
 ****************************************************************************/

void CueStack::setFadeInSpeed(uint ms)
{
    m_fadeInSpeed = ms;
}

uint CueStack::fadeInSpeed() const
{
    return m_fadeInSpeed;
}

void CueStack::setFadeOutSpeed(uint ms)
{
    m_fadeOutSpeed = ms;
}

uint CueStack::fadeOutSpeed() const
{
    return m_fadeOutSpeed;
}

void CueStack::setDuration(uint ms)
{
    m_duration = ms;
}

uint CueStack::duration() const
{
    return m_duration;
}

/****************************************************************************
 * Cues
 ****************************************************************************/

void CueStack::appendCue(const Cue& c)
{
    m_cues.append(c);
}

void CueStack::removeCue(int index)
{
    if (index >= 0 && index < m_cues.size())
    {
        m_cues.removeAt(index);
        if (index < m_currentIndex)
            m_currentIndex--;
    }
}

QList <Cue> CueStack::cues() const
{
    return m_cues;
}

void CueStack::setCurrentIndex(int index)
{
    m_currentIndex = CLAMP(index, -1, m_cues.size() - 1);
}

int CueStack::currentIndex() const
{
    return m_currentIndex;
}

void CueStack::previousCue()
{
    m_previous = true;
    if (isRunning() == false)
        start();
}

void CueStack::nextCue()
{
    m_next = true;
    if (isRunning() == false)
        start();
}

/****************************************************************************
 * Running
 ****************************************************************************/

void CueStack::start()
{
    qDebug() << Q_FUNC_INFO;

    m_running = true;
}

void CueStack::stop()
{
    qDebug() << Q_FUNC_INFO;

    m_running = false;
}

bool CueStack::isRunning() const
{
    return m_running;
}

void CueStack::adjustIntensity(qreal fraction)
{
    m_intensity = fraction;
    if (m_fader != NULL)
        m_fader->adjustIntensity(fraction);
}

qreal CueStack::intensity() const
{
    return m_intensity;
}

/****************************************************************************
 * Writing
 ****************************************************************************/

bool CueStack::isStarted() const
{
    if (m_fader != NULL)
        return true;
    else
        return false;
}

void CueStack::preRun()
{
    qDebug() << Q_FUNC_INFO;

    Q_ASSERT(m_fader == NULL);
    m_fader = new GenericFader(doc());
    m_fader->adjustIntensity(intensity());
    m_elapsed = 0;
    emit started();
}

void CueStack::write(UniverseArray* ua)
{
    Q_ASSERT(m_fader != NULL);
    Q_ASSERT(ua != NULL);

    if (m_cues.size() == 0)
        return;

    if (m_previous == true)
    {
        // previousCue() was requested by user
        m_elapsed = 0;
        switchCue(previous(), ua);
        m_previous = false;
        emit currentCueChanged(m_currentIndex);
    }
    else if (m_next == true)
    {
        // nextCue() was requested by user
        m_elapsed = 0;
        switchCue(next(), ua);
        m_next = false;
        emit currentCueChanged(m_currentIndex);
    }
/*
    else if (m_elapsed >= duration())
    {
        // Duration expired
        m_elapsed = 0;
        switchCue(next(), ua);
        emit currentCueChanged(m_currentIndex);
    }
*/
    m_fader->write(ua);

    m_elapsed += MasterTimer::tick();
}

void CueStack::postRun(MasterTimer* timer)
{
    qDebug() << Q_FUNC_INFO;

    Q_ASSERT(timer != NULL);
    Q_ASSERT(m_fader != NULL);

    // Bounce all intensity channels to MasterTimer's fader for zeroing
    QHashIterator <FadeChannel,FadeChannel> it(m_fader->channels());
    while (it.hasNext() == true)
    {
        it.next();
        FadeChannel fc = it.value();

        if (fc.group(doc()) == QLCChannel::Intensity)
        {
            fc.setStart(fc.current(intensity()));
            fc.setTarget(0);
            fc.setElapsed(0);
            fc.setReady(false);
            fc.setFadeTime(fadeOutSpeed());
            timer->fader()->add(fc);
        }
    }

    delete m_fader;
    m_fader = NULL;

    m_currentIndex = -1;
    emit currentCueChanged(m_currentIndex);
    emit stopped();
}

int CueStack::previous()
{
    qDebug() << Q_FUNC_INFO;

    Q_ASSERT(m_fader != NULL);

    m_currentIndex--;
    if (m_currentIndex < 0)
        m_currentIndex = m_cues.size() - 1;

    return m_currentIndex;
}

int CueStack::next()
{
    qDebug() << Q_FUNC_INFO;

    Q_ASSERT(m_fader != NULL);
    if (m_cues.size() == 0)
        return -1;

    m_currentIndex++;
    if (m_currentIndex >= m_cues.size())
        m_currentIndex = 0;

    return m_currentIndex;
}

void CueStack::switchCue(int index, const UniverseArray* ua)
{
    qDebug() << Q_FUNC_INFO;

    QHashIterator <FadeChannel,FadeChannel> it(m_fader->channels());
    while (it.hasNext() == true)
    {
        it.next();
        FadeChannel fc = it.value();
        if (fc.group(doc()) == QLCChannel::Intensity)
        {
            fc.setStart(fc.current());
            fc.setTarget(0);
            fc.setElapsed(0);
            fc.setReady(false);
            fc.setFadeTime(fadeOutSpeed());
            m_fader->add(fc);
        }
    }

    if (index < 0 || index >= m_cues.size())
        return;

    const Cue& cue = m_cues[index];
    QHashIterator <uint,uchar> cit(cue.values());
    while (cit.hasNext() == true)
    {
        cit.next();
        FadeChannel fc;

        fc.setFixture(Fixture::invalidId());
        fc.setChannel(cit.key());
        fc.setTarget(cit.value());
        fc.setElapsed(0);
        fc.setReady(false);
        fc.setFadeTime(fadeInSpeed());
        insertStartValue(fc, ua);
        m_fader->add(fc);
    }
}

void CueStack::insertStartValue(FadeChannel& fc, const UniverseArray* ua)
{
    qDebug() << Q_FUNC_INFO;

    const QHash <FadeChannel,FadeChannel>& channels(m_fader->channels());
    if (channels.contains(fc) == true)
    {
        // GenericFader contains the channel so grab its current
        // value as the new starting value to get a smoother fade
        FadeChannel existing = channels[fc];
        fc.setStart(existing.current());
        fc.setCurrent(fc.start());
    }
    else
    {
        // GenericFader didn't have the channel. Grab the starting value from UniverseArray.
        quint32 address = fc.address(doc());
        if (fc.group(doc()) != QLCChannel::Intensity)
            fc.setStart(ua->preGMValues()[address]);
        else
            fc.setStart(0); // HTP channels must start at zero
        fc.setCurrent(fc.start());
    }
}
