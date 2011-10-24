/*
  Q Light Controller
  chaserrunner.cpp

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

#include "universearray.h"
#include "chaserrunner.h"
#include "genericfader.h"
#include "mastertimer.h"
#include "fadechannel.h"
#include "chaserstep.h"
#include "qlcmacros.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"

ChaserRunner::ChaserRunner(Doc* doc, QList <ChaserStep> steps,
                           uint fadeInSpeed, uint fadeOutSpeed, uint duration,
                           Function::Direction direction, Function::RunOrder runOrder,
                           qreal intensity, QObject* parent, int startIndex)
    : QObject(parent)
    , m_doc(doc)
    , m_steps(steps)
    , m_originalDirection(direction)
    , m_runOrder(runOrder)

    , m_fadeInSpeed(fadeInSpeed)
    , m_fadeOutSpeed(fadeOutSpeed)
    , m_duration(duration)

    , m_autoStep(true)

    , m_direction(direction)
    , m_currentFunction(NULL)
    , m_elapsed(0)
    , m_next(false)
    , m_previous(false)
    , m_currentStep(0)
    , m_newCurrent(-1)
    , m_intensity(intensity)
{
    reset();
    if (startIndex != -1)
        m_currentStep = startIndex;
}

ChaserRunner::~ChaserRunner()
{
}

/****************************************************************************
 * Speed adjustment
 ****************************************************************************/

void ChaserRunner::setDuration(uint ms)
{
    m_duration = ms;
}

/****************************************************************************
 * Automatic stepping
 ****************************************************************************/

void ChaserRunner::setAutoStep(bool autoStep)
{
    m_autoStep = autoStep;
}

bool ChaserRunner::isAutoStep() const
{
    return m_autoStep;
}

/****************************************************************************
 * Step control
 ****************************************************************************/

void ChaserRunner::next()
{
    m_next = true;
    m_previous = false;
}

void ChaserRunner::previous()
{
    m_next = false;
    m_previous = true;
}

void ChaserRunner::setCurrentStep(int step)
{
    if (step >= 0 && step < m_steps.size())
    {
        m_newCurrent = step;
        m_next = false;
        m_previous = false;
    }
}

int ChaserRunner::currentStep() const
{
    return m_currentStep;
}

void ChaserRunner::reset()
{
    // Restore original direction since Ping-Pong switches m_direction
    m_direction = m_originalDirection;

    if (m_direction == Function::Backward)
        m_currentStep = m_steps.size() - 1;
    else
        m_currentStep = 0;

    m_elapsed = 0;
    m_next = false;
    m_previous = false;
    m_currentFunction = NULL;
}

/****************************************************************************
 * Intensity
 ****************************************************************************/

void ChaserRunner::adjustIntensity(qreal fraction)
{
    m_intensity = CLAMP(fraction, qreal(0.0), qreal(1.0));
    if (m_currentFunction != NULL)
        m_currentFunction->adjustIntensity(m_intensity);
}

/****************************************************************************
 * Running
 ****************************************************************************/

bool ChaserRunner::write(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(universes);

    // Nothing to do
    if (m_steps.size() == 0)
        return false;

    if (m_newCurrent != -1)
    {
        // Manually-set current step
        m_currentStep = m_newCurrent;
        m_newCurrent = -1;

        // No need to do roundcheck here, since manually-set steps are
        // always within m_steps limits.

        m_elapsed = MasterTimer::tick();
        switchFunctions(timer);
        emit currentStepChanged(m_currentStep);
    }
    else if (m_elapsed == 0)
    {
        // First step
        m_elapsed = MasterTimer::tick();
        switchFunctions(timer);
        emit currentStepChanged(m_currentStep);
    }
    else if ((isAutoStep() && m_elapsed >= m_duration) || m_next == true || m_previous == true)
    {
        // Next step
        if (m_direction == Function::Forward)
        {
            // "Previous" for a forwards chaser is -1
            if (m_previous == true)
                m_currentStep--;
            else
                m_currentStep++;
        }
        else
        {
            // "Previous" for a backwards scene is +1
            if (m_previous == true)
                m_currentStep++;
            else
                m_currentStep--;
        }

        if (roundCheck() == false)
            return false;

        m_elapsed = MasterTimer::tick();
        m_next = false;
        m_previous = false;

        switchFunctions(timer);
        emit currentStepChanged(m_currentStep);
    }
    else
    {
        // Current step. UINT_MAX is the maximum hold time.
        if (m_elapsed < UINT_MAX)
            m_elapsed += MasterTimer::tick();
    }

    return true;
}

void ChaserRunner::postRun(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(universes);
    Q_UNUSED(timer);

    if (m_currentFunction != NULL && m_currentFunction->stopped() == false)
        m_currentFunction->stop();
    m_currentFunction = NULL;
}

bool ChaserRunner::roundCheck()
{
    if (m_currentStep < m_steps.size() && m_currentStep >= 0)
        return true; // In the middle of steps. No need to go any further.

    if (m_runOrder == Function::SingleShot)
    {
        if (m_direction == Function::Forward)
        {
            if (m_currentStep >= m_steps.size())
                return false; // Forwards SingleShot has been completed.
            else
                m_currentStep = 0; // No wrapping
        }
        else // Backwards
        {
            if (m_currentStep < 0)
                return false; // Backwards SingleShot has been completed.
            else
                m_currentStep = m_steps.size() - 1; // No wrapping
        }
    }
    else if (m_runOrder == Function::Loop)
    {
        if (m_direction == Function::Forward)
        {
            if (m_currentStep >= m_steps.size())
                m_currentStep = 0;
            else
                m_currentStep = m_steps.size() - 1;
        }
        else // Backwards
        {
            if (m_currentStep < 0)
                m_currentStep = m_steps.size() - 1;
            else
                m_currentStep = 0;
        }
    }
    else // Ping Pong
    {
        // Change direction, but don't run the first/last step twice.
        if (m_direction == Function::Forward)
        {
            if (m_currentStep >= m_steps.size())
                m_currentStep = 1;
            else
                m_currentStep = m_steps.size() - 2;
            m_direction = Function::Backward;
        }
        else // Backwards
        {
            if (m_currentStep < 0)
                m_currentStep = m_steps.size() - 2;
            else
                m_currentStep = 1;
            m_direction = Function::Forward;
        }

        // Make sure we don't go beyond limits.
        m_currentStep = CLAMP(m_currentStep, 0, m_steps.size() - 1);
    }

    // Let's continue
    return true;
}

void ChaserRunner::switchFunctions(MasterTimer* timer)
{
    if (m_currentFunction != NULL)
        m_currentFunction->stop();

    ChaserStep step(m_steps.at(m_currentStep));
    m_currentFunction = m_doc->function(step.fid);
    if (m_currentFunction != NULL && m_currentFunction->stopped() == true)
    {
        // Set intensity before starting the function. Otherwise the intensity
        // might momentarily jump too high.
        m_currentFunction->adjustIntensity(m_intensity);
        m_currentFunction->start(timer, true, m_fadeInSpeed, m_fadeOutSpeed, m_duration);
    }
}
