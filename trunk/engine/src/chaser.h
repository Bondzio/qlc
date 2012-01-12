/*
  Q Light Controller
  chaser.h

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

#ifndef CHASER_H
#define CHASER_H

#include <QMutex>
#include <QList>

#include "function.h"

class QFile;
class QString;
class ChaserStep;
class MasterTimer;
class ChaserRunner;
class QDomDocument;

/**
 * Chaser is a meta-function; it consists of other functions that are run in a
 * sequential order. Chaser contains information only on the running order,
 * direction and interval applied for its member functions. Therefore, a chaser
 * itself is nothing without other functions (more than usually, scenes) as its
 * members. If a member function's behaviour is changed, those changes are also
 * applied automatically to those chasers that hold the changed function(s) as
 * their members.
 */
class Chaser : public Function
{
    Q_OBJECT
    Q_DISABLE_COPY(Chaser)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    Chaser(Doc* doc);
    virtual ~Chaser();

private:
    quint32 m_legacyHoldBus;

    /*********************************************************************
     * Copying
     *********************************************************************/
public:
    /** @reimpl */
    Function* createCopy(Doc* doc);

    /** Copy the contents for this function from another function */
    bool copyFrom(const Function* function);

    /*********************************************************************
     * Chaser contents
     *********************************************************************/
public:
    /**
     * Add the given step to the chaser, either at the specified index, given
     * that 0 <= index < size or at the end if size < index < 0. The same
     * function can exist any number of times in a chaser. No checks for the
     * function's validity are made at this point, except that the chaser's own
     * ID cannot be added (i.e. a chaser cannot be its own direct member).
     *
     * @param step The step to add
     * @param index Insertion point. -1 to append.
     */
    bool addStep(const ChaserStep& step, int index = -1);

    /**
     * Remove a function from the given step index. If the given index is
     * out of bounds, this returns false.
     *
     * @param index The index number to remove
     * @return true if successful, otherwise false
     */
    bool removeStep(int index);

    /**
     * Clear the chaser's list of steps
     */
    void clear();

    /**
     * Get the chaser's list of steps
     *
     * @return List of function Chaser Steps
     */
    QList <ChaserStep> steps() const;

public slots:
    /**
     * Catches Doc::functionRemoved() so that destroyed members can be
     * removed immediately. This method removes all occurrences of the
     * given function ID, while removeStep() only removes one function ID
     * at the given index.
     *
     * @param fid The ID of the function that was removed
     */
    void slotFunctionRemoved(quint32 fid);

private:
    QList <ChaserStep> m_steps;
    QMutex m_stepListMutex;

    /*********************************************************************
     * Save & Load
     *********************************************************************/
public:
    /** Save this function to an XML document */
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

    /** Load this function contents from an XML document */
    bool loadXML(const QDomElement& root);

    /** @reimp */
    void postLoad();

    /*********************************************************************
     * Speed
     *********************************************************************/
public:
    /** @reimpl */
    void tap();

    /*********************************************************************
     * Running
     *********************************************************************/
public:
    /** @reimpl */
    void preRun(MasterTimer* timer);

    /** @reimpl */
    void write(MasterTimer* timer, UniverseArray* universes);

    /** @reimpl */
    void postRun(MasterTimer* timer, UniverseArray* universes);

private:
    ChaserRunner* m_runner;

    /*************************************************************************
     * Intensity
     *************************************************************************/
public:
    /** @reimpl */
    void adjustIntensity(qreal fraction);
};

#endif
