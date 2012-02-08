/*
  Q Light Controller
  chaserrunner.h

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

#ifndef CHASERRUNNER_H
#define CHASERRUNNER_H

#include <QList>
#include <QMap>

#include "function.h"

class UniverseArray;
class FadeChannel;
class ChaserStep;
class Function;
class Doc;

class ChaserRunner : public QObject
{
    Q_OBJECT

public:
    ChaserRunner(Doc* doc, QList <ChaserStep> steps,
                 uint fadeInSpeed, uint fadeOutSpeed, uint duration,
                 Function::Direction direction, Function::RunOrder runOrder,
                 qreal intensity = 1.0, QObject* parent = NULL, int startIndex = -1);
    ~ChaserRunner();

private:
    const Doc* m_doc;
    const QList <ChaserStep> m_steps;
    const Function::Direction m_originalDirection;
    const Function::RunOrder m_runOrder;

    /************************************************************************
     * Speed adjustment
     ************************************************************************/
public:
    /**
     * Adjust the step duration. Can be used to adjust run-time duration,
     * for example when tapping.
     */
    void setDuration(uint ms);

    /**
     * Get the currently active fade in value (either a global setting or
     * associated with the current step).
     */
    uint currentFadeIn() const;

    /**
     * Get the currently active fade out value (either a global setting or
     * associated with the current step).
     */
    uint currentFadeOut() const;

    /**
     * Get the currently active duration value (either a global setting or
     * associated with the current step).
     */
    uint currentDuration() const;

private:
    uint m_fadeInSpeed;
    uint m_fadeOutSpeed;
    uint m_duration;

    /************************************************************************
     * Automatic stepping
     ************************************************************************/
public:
    /**
     * Enables automatic stepping if $auto is true; otherwise automatic
     * stepping is disabled and the only way to skip to next/previous step is
     * thru next() and previous() methods.
     *
     * @param auto Enable/disable automatic stepping
     */
    void setAutoStep(bool autoStep);

    /**
     * Check, if automatic stepping is enabled (default = true).
     *
     * @return true if automatic stepping is enabled, otherwise false.
     */
    bool isAutoStep() const;

private:
    bool m_autoStep;

    /************************************************************************
     * Step control
     ************************************************************************/
public:
    /**
     * Skip to the next scene, obeying direction and run order settings.
     */
    void next();

    /**
     * Skip to the previous scene, obeying direction and run order settings.
     */
    void previous();

    /**
     * Set the current step number.
     *
     * @param step Step number to set
     */
    void setCurrentStep(int step);

    /**
     * Get the current step number.
     *
     * @return Current step number
     */
    int currentStep() const;

    /**
     * Reset the runner to a state where nothing has been run yet.
     */
    void reset();

signals:
    /** Tells that the current step number has changed. */
    void currentStepChanged(int stepNumber);

private:
    Function::Direction m_direction; //! Run-time direction (reversed by ping-pong)
    Function* m_currentFunction;     //! Currently active function
    uint m_elapsed;                  //! Elapsed milliseconds
    bool m_next;                     //! If true, skips to the next step when write is called
    bool m_previous;                 //! If true, skips to the previous step when write is called
    int m_currentStep;               //! Current step in m_steps
    int m_newCurrent;                //! Manually set the current step

    /************************************************************************
     * Intensity
     ************************************************************************/
public:
    /**
     * Adjust the intensities of chaser steps.
     */
    void adjustIntensity(qreal fraction);

private:
    qreal m_intensity;

    /************************************************************************
     * Running
     ************************************************************************/
public:
    /**
     * Call this from the parent function's write() method to run the steps.
     *
     * @param timer The MasterTimer that runs the show
     * @param universes DMX address space
     * @return true if the chaser should continue, otherwise false
     */
    bool write(MasterTimer* timer, UniverseArray* universes);

    /**
     * Perform postRun operations. Call this from the parent function's postRun().
     *
     * @param timer The MasterTimer that runs the show
     * @param universes DMX address space
     */
    void postRun(MasterTimer* timer, UniverseArray* universes);

private:
    /** Ran at each end of m_steps. Returns false only when SingleShot has been
        completed. */
    bool roundCheck();

    /**
     * Stop the previous function (if applicable) and start a new one (current).
     *
     * @param timer The MasterTimer that runs the functions
     */
    void switchFunctions(MasterTimer* timer);
};

#endif
