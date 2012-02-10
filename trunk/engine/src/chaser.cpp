/*
  Q Light Controller
  chaser.cpp

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
#include <QFile>
#include <QtXml>

#include "qlcfixturedef.h"
#include "qlcfile.h"

#include "universearray.h"
#include "chaserrunner.h"
#include "mastertimer.h"
#include "chaserstep.h"
#include "function.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"
#include "bus.h"

#define KXMLQLCChaserGlobalSpeed "GlobalSpeed"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Chaser::Chaser(Doc* doc)
    : Function(doc, Function::Chaser)
    , m_legacyHoldBus(Bus::invalid())
    , m_globalFadeIn(true)
    , m_globalFadeOut(true)
    , m_globalDuration(true)
    , m_runner(NULL)
{
    setName(tr("New Chaser"));

    // Listen to member Function removals
    connect(doc, SIGNAL(functionRemoved(quint32)),
            this, SLOT(slotFunctionRemoved(quint32)));
}

Chaser::~Chaser()
{
}

/*****************************************************************************
 * Copying
 *****************************************************************************/

Function* Chaser::createCopy(Doc* doc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new Chaser(doc);
    if (copy->copyFrom(this) == false || doc->addFunction(copy) == false)
    {
        delete copy;
        copy = NULL;
    }

    return copy;
}

bool Chaser::copyFrom(const Function* function)
{
    const Chaser* chaser = qobject_cast<const Chaser*> (function);
    if (chaser == NULL)
        return false;

    m_steps.clear();
    m_steps = chaser->m_steps;
    m_globalFadeIn = chaser->m_globalFadeIn;
    m_globalFadeOut = chaser->m_globalFadeOut;
    m_globalDuration = chaser->m_globalDuration;

    return Function::copyFrom(function);
}

/*****************************************************************************
 * Contents
 *****************************************************************************/

bool Chaser::addStep(const ChaserStep& step, int index)
{
    if (step.fid != this->id())
    {
        m_stepListMutex.lock();
        if (index < 0)
            m_steps.append(step);
        else if (index <= m_steps.size())
            m_steps.insert(index, step);
        m_stepListMutex.unlock();

        emit changed(this->id());
        return true;
    }
    else
    {
        return false;
    }
}

bool Chaser::removeStep(int index)
{
    if (index >= 0 && index < m_steps.size())
    {
        m_stepListMutex.lock();
        m_steps.removeAt(index);
        m_stepListMutex.unlock();

        emit changed(this->id());
        return true;
    }
    else
    {
        return false;
    }
}

bool Chaser::replaceStep(const ChaserStep& step, int index)
{
    if (index >= 0 && index < m_steps.size())
    {
        m_stepListMutex.lock();
        m_steps[index] = step;
        m_stepListMutex.unlock();

        emit changed(this->id());
        return true;
    }
    else
    {
        return false;
    }
}

void Chaser::clear()
{
    m_steps.clear();
    emit changed(this->id());
}

QList <ChaserStep> Chaser::steps() const
{
    return m_steps;
}

void Chaser::slotFunctionRemoved(quint32 fid)
{
    m_stepListMutex.lock();
    int count = m_steps.removeAll(ChaserStep(fid));
    m_stepListMutex.unlock();

    if (count > 0)
        emit changed(this->id());
}

/*****************************************************************************
 * Global speed override
 *****************************************************************************/

void Chaser::setGlobalFadeIn(bool set)
{
    m_globalFadeIn = set;
    emit changed(this->id());
}

bool Chaser::isGlobalFadeIn() const
{
    return m_globalFadeIn;
}

void Chaser::setGlobalFadeOut(bool set)
{
    m_globalFadeOut = set;
    emit changed(this->id());
}

bool Chaser::isGlobalFadeOut() const
{
    return m_globalFadeOut;
}

void Chaser::setGlobalDuration(bool set)
{
    m_globalDuration = set;
    emit changed(this->id());
}

bool Chaser::isGlobalDuration() const
{
    return m_globalDuration;
}

/*****************************************************************************
 * Save & Load
 *****************************************************************************/

bool Chaser::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement root;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    /* Function tag */
    root = doc->createElement(KXMLQLCFunction);
    wksp_root->appendChild(root);

    root.setAttribute(KXMLQLCFunctionID, id());
    root.setAttribute(KXMLQLCFunctionType, Function::typeToString(type()));
    root.setAttribute(KXMLQLCFunctionName, name());

    /* Speed */
    saveXMLSpeed(doc, &root);

    /* Direction */
    saveXMLDirection(doc, &root);

    /* Run order */
    saveXMLRunOrder(doc, &root);

    /* Global speed */
    QDomElement spd = doc->createElement(KXMLQLCChaserGlobalSpeed);
    spd.setAttribute(KXMLQLCFunctionSpeedFadeIn, isGlobalFadeIn() ? KXMLQLCTrue : KXMLQLCFalse);
    spd.setAttribute(KXMLQLCFunctionSpeedFadeOut, isGlobalFadeOut() ? KXMLQLCTrue : KXMLQLCFalse);
    spd.setAttribute(KXMLQLCFunctionSpeedDuration, isGlobalDuration() ? KXMLQLCTrue : KXMLQLCFalse);
    root.appendChild(spd);

    /* Steps */
    int stepNumber = 0;
    QListIterator <ChaserStep> it(m_steps);
    while (it.hasNext() == true)
    {
        ChaserStep step(it.next());
        step.saveXML(doc, &root, stepNumber++);
    }

    return true;
}

bool Chaser::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root.attribute(KXMLQLCFunctionType) != typeToString(Function::Chaser))
    {
        qWarning() << Q_FUNC_INFO << root.attribute(KXMLQLCFunctionType)
                   << "is not a chaser";
        return false;
    }

    /* Load chaser contents */
    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();

        if (tag.tagName() == KXMLQLCBus)
        {
            m_legacyHoldBus = tag.text().toUInt();
        }
        else if (tag.tagName() == KXMLQLCFunctionSpeed)
        {
            loadXMLSpeed(tag);
        }
        else if (tag.tagName() == KXMLQLCFunctionDirection)
        {
            loadXMLDirection(tag);
        }
        else if (tag.tagName() == KXMLQLCFunctionRunOrder)
        {
            loadXMLRunOrder(tag);
        }
        else if (tag.tagName() == KXMLQLCChaserGlobalSpeed)
        {
            setGlobalFadeIn(tag.attribute(KXMLQLCFunctionSpeedFadeIn) == KXMLQLCTrue);
            setGlobalFadeOut(tag.attribute(KXMLQLCFunctionSpeedFadeOut) == KXMLQLCTrue);
            setGlobalDuration(tag.attribute(KXMLQLCFunctionSpeedDuration) == KXMLQLCTrue);
        }
        else if (tag.tagName() == KXMLQLCFunctionStep)
        {
            //! @todo stepNumber is useless if the steps are in the wrong order
            ChaserStep step;
            int stepNumber = -1;
            if (step.loadXML(tag, stepNumber) == true)
            {
                if (stepNumber >= m_steps.size())
                    m_steps.append(step);
                else
                    m_steps.insert(stepNumber, step);
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown chaser tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

void Chaser::postLoad()
{
    if (m_legacyHoldBus != Bus::invalid())
    {
        quint32 value = Bus::instance()->value(m_legacyHoldBus);
        setDuration((value / MasterTimer::frequency()) * 1000);
    }

    QMutableListIterator <ChaserStep> it(m_steps);
    while (it.hasNext() == true)
    {
        ChaserStep step(it.next());
        Function* function = doc()->function(step.fid);
        if (function == NULL)
            it.remove();
    }
}

/*****************************************************************************
 * Speed
 *****************************************************************************/

void Chaser::tap()
{
    Function::tap();
    if (m_runner != NULL)
    {
        m_runner->setDuration(duration());
        m_runner->next();
    }
}

/*****************************************************************************
 * Running
 *****************************************************************************/

ChaserRunner* Chaser::createRunner(Chaser* self, Doc* doc)
{
    if (self == NULL || doc == NULL)
        return NULL;

    // Use global speeds only if they have been enabled
    uint fadeIn = Function::defaultSpeed();
    if (self->isGlobalFadeIn() == true)
        fadeIn = self->fadeInSpeed();
    uint fadeOut = Function::defaultSpeed();
    if (self->isGlobalFadeOut() == true)
        fadeOut = self->fadeOutSpeed();
    uint dur = Function::defaultSpeed();
    if (self->isGlobalDuration() == true)
        dur = self->duration();

    // These override* variables are set only if a chaser is started by another function
    if (self->overrideFadeInSpeed() != defaultSpeed())
        fadeIn = self->overrideFadeInSpeed();
    if (self->overrideFadeOutSpeed() != defaultSpeed())
        fadeOut = self->overrideFadeOutSpeed();
    if (self->overrideDuration() != defaultSpeed())
        dur = self->overrideDuration();

    self->m_stepListMutex.lock();
    ChaserRunner* runner = new ChaserRunner(doc,
                                            self->steps(),
                                            fadeIn,
                                            fadeOut,
                                            dur,
                                            self->direction(),
                                            self->runOrder(),
                                            self->intensity(),
                                            self);
    self->m_stepListMutex.unlock();

    return runner;
}

void Chaser::preRun(MasterTimer* timer)
{
/*
    // Use global speeds only if they have been enabled
    uint fadeIn = Function::defaultSpeed();
    if (isGlobalFadeIn() == true)
        fadeIn = fadeInSpeed();
    uint fadeOut = Function::defaultSpeed();
    if (isGlobalFadeOut() == true)
        fadeOut = fadeOutSpeed();
    uint dur = Function::defaultSpeed();
    if (isGlobalDuration() == true)
        dur = duration();

    // These override* variables are set only if a chaser is started by another function
    if (overrideFadeInSpeed() != defaultSpeed())
        fadeIn = overrideFadeInSpeed();
    if (overrideFadeOutSpeed() != defaultSpeed())
        fadeOut = overrideFadeOutSpeed();
    if (overrideDuration() != defaultSpeed())
        dur = overrideDuration();

    m_stepListMutex.lock();
    m_runner = new ChaserRunner(doc(), steps(), fadeIn, fadeOut, dur,
                                direction(), runOrder(), intensity());
    m_stepListMutex.unlock();
*/
    Q_ASSERT(m_runner == NULL);
    m_runner = createRunner(this, doc());
    Function::preRun(timer);
}

void Chaser::write(MasterTimer* timer, UniverseArray* universes)
{
    Q_ASSERT(m_runner != NULL);

    if (m_runner->write(timer, universes) == false)
        stop();

    incrementElapsed();
}

void Chaser::postRun(MasterTimer* timer, UniverseArray* universes)
{
    m_runner->postRun(timer, universes);

    Q_ASSERT(m_runner != NULL);
    delete m_runner;
    m_runner = NULL;

    Function::postRun(timer, universes);
}

/*****************************************************************************
 * Intensity
 *****************************************************************************/

void Chaser::adjustIntensity(qreal fraction)
{
    if (m_runner != NULL)
        m_runner->adjustIntensity(fraction);
    Function::adjustIntensity(fraction);
}
