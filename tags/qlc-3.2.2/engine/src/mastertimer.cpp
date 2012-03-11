/*
  Q Light Controller
  mastertimer.cpp

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

#include <QDebug>

#ifdef WIN32
#   include "mastertimer-win32.h"
#else
#   include "mastertimer-unix.h"
#endif

#include "universearray.h"
#include "genericfader.h"
#include "mastertimer.h"
#include "outputmap.h"
#include "dmxsource.h"
#include "qlcmacros.h"
#include "function.h"

/** The timer tick frequency in Hertz */
const uint MasterTimer::s_frequency = 50;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

MasterTimer::MasterTimer(QObject* parent, OutputMap* outputMap)
    : QObject(parent)
    , m_outputMap(outputMap)
    , m_stopAllFunctions(false)
    , m_fader(new GenericFader)
    , d_ptr(new MasterTimerPrivate(this))
{
    Q_ASSERT(parent != NULL);
    Q_ASSERT(outputMap != NULL);
    Q_ASSERT(d_ptr != NULL);
}

MasterTimer::~MasterTimer()
{
    if (d_ptr->isRunning() == true)
        stop();

    delete d_ptr;
    d_ptr = NULL;
}

void MasterTimer::start()
{
    Q_ASSERT(d_ptr != NULL);
    d_ptr->start();
}

void MasterTimer::stop()
{
    Q_ASSERT(d_ptr != NULL);
    stopAllFunctions();
    d_ptr->stop();
}

uint MasterTimer::frequency()
{
    return s_frequency;
}

uint MasterTimer::tick()
{
    return uint(double(1000) / double(s_frequency));
}

void MasterTimer::timerTick()
{
    UniverseArray* universes = outputMap()->claimUniverses();
    universes->zeroIntensityChannels();

    timerTickFunctions(universes);
    timerTickDMXSources(universes);
    timerTickFader(universes);

    outputMap()->releaseUniverses();
    outputMap()->dumpUniverses();
}

OutputMap* MasterTimer::outputMap() const
{
    return m_outputMap;
}

/*****************************************************************************
 * Functions
 *****************************************************************************/

void MasterTimer::startFunction(Function* function, bool initiatedByOtherFunction)
{
    if (function == NULL)
        return;

    m_functionListMutex.lock();
    if (m_functionList.contains(function) == false)
    {
        m_functionList.append(function);
        function->setInitiatedByOtherFunction(initiatedByOtherFunction);
    }
    m_functionListMutex.unlock();

    emit functionListChanged();
}

void MasterTimer::stopAllFunctions()
{
    m_stopAllFunctions = true;

    /* Wait until all functions have been stopped */
    while (runningFunctions() > 0)
    {
#ifdef WIN32
        Sleep(10);
#else
        usleep(10000);
#endif
    }

    /* Remove all generic fader's channels */
    m_functionListMutex.lock();
    m_dmxSourceListMutex.lock();
    fader()->removeAll();
    m_dmxSourceListMutex.unlock();
    m_functionListMutex.unlock();

    m_stopAllFunctions = false;
}

int MasterTimer::runningFunctions() const
{
    return m_functionList.size();
}

void MasterTimer::timerTickFunctions(UniverseArray* universes)
{
    /* Lock before accessing the running functions list. */
    m_functionListMutex.lock();
    for (int i = 0; i < m_functionList.size(); i++)
    {
        Function* function = m_functionList.at(i);

        /* No need to access function list on this round anymore */
        m_functionListMutex.unlock();

        if (function != NULL)
        {
            if (function->elapsed() == 0)
                function->preRun(this);

            /* Check for pre-conditions before getting data */
            if (function->stopped() == true || m_stopAllFunctions == true)
            {
                /* Function should be stopped instead */
                m_functionListMutex.lock();
                m_functionList.removeAt(i);
                function->postRun(this, universes);
                m_functionListMutex.unlock();
                emit functionListChanged();
            }
            else
            {
                /* Run normally: get function data */
                function->write(this, universes);
            }
        }

        /* Lock function list for the next round. */
        m_functionListMutex.lock();
    }

    /* No more functions. Get out and wait for next timer event. */
    m_functionListMutex.unlock();
}

/****************************************************************************
 * DMX Sources
 ****************************************************************************/

void MasterTimer::registerDMXSource(DMXSource* source)
{
    Q_ASSERT(source != NULL);

    m_dmxSourceListMutex.lock();
    if (m_dmxSourceList.contains(source) == false)
        m_dmxSourceList.append(source);
    m_dmxSourceListMutex.unlock();
}

void MasterTimer::unregisterDMXSource(DMXSource* source)
{
    Q_ASSERT(source != NULL);

    m_dmxSourceListMutex.lock();
    m_dmxSourceList.removeAll(source);
    m_dmxSourceListMutex.unlock();
}

void MasterTimer::timerTickDMXSources(UniverseArray* universes)
{
    /* Lock before accessing the running functions list. */
    m_dmxSourceListMutex.lock();
    for (int i = 0; i < m_dmxSourceList.size(); i++)
    {
        DMXSource* source = m_dmxSourceList.at(i);
        Q_ASSERT(source != NULL);

        /* No need to access the list on this round anymore. */
        m_dmxSourceListMutex.unlock();

        /* Get DMX data from the source */
        source->writeDMX(this, universes);

        /* Lock for the next round. */
        m_dmxSourceListMutex.lock();
    }

    /* No more sources. Get out and wait for next timer event. */
    m_dmxSourceListMutex.unlock();
}

/****************************************************************************
 * Generic Fader
 ****************************************************************************/

GenericFader* MasterTimer::fader() const
{
    return m_fader;
}

void MasterTimer::timerTickFader(UniverseArray* universes)
{
    m_functionListMutex.lock();
    m_dmxSourceListMutex.lock();

    fader()->write(universes);

    m_dmxSourceListMutex.unlock();
    m_functionListMutex.unlock();
}
