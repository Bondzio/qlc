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
#include "function.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"
#include "bus.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Chaser::Chaser(Doc* doc) : Function(doc, Function::Chaser)
{
    m_runTimeDirection = Forward;
    m_runTimePosition = 0;
    m_runner = NULL;
    m_legacyHoldBus = Bus::defaultHold();

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

    return Function::copyFrom(function);
}

/*****************************************************************************
 * Contents
 *****************************************************************************/

bool Chaser::addStep(quint32 id, int index)
{
    if (id != this->id())
    {
        if (index < 0)
            m_steps.append(id);
        else if (index <= m_steps.size())
            m_steps.insert(index, id);

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
        m_steps.removeAt(index);
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

QList <quint32> Chaser::steps() const
{
    return m_steps;
}

QList <Function*> Chaser::stepFunctions() const
{
    QList <Function*> list;
    QListIterator <quint32> it(m_steps);
    while (it.hasNext() == true)
    {
        Function* function = doc()->function(it.next());
        if (function != NULL)
            list << function;
    }

    return list;
}

void Chaser::slotFunctionRemoved(quint32 fid)
{
    m_steps.removeAll(fid);
}

/*****************************************************************************
 * Save & Load
 *****************************************************************************/

bool Chaser::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement root;
    QDomElement tag;
    QDomText text;
    QString str;
    int i = 0;

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

    /* Steps */
    QListIterator <quint32> it(m_steps);
    while (it.hasNext() == true)
    {
        /* Step tag */
        tag = doc->createElement(KXMLQLCFunctionStep);
        root.appendChild(tag);

        /* Step number */
        tag.setAttribute(KXMLQLCFunctionNumber, i++);

        /* Step Function ID */
        str.setNum(it.next());
        text = doc->createTextNode(str);
        tag.appendChild(text);
    }

    return true;
}

bool Chaser::loadXML(const QDomElement* root)
{
    QDomNode node;
    QDomElement tag;

    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root->attribute(KXMLQLCFunctionType) != typeToString(Function::Chaser))
    {
        qWarning() << Q_FUNC_INFO << root->attribute(KXMLQLCFunctionType)
                   << "is not a chaser";
        return false;
    }

    /* Load chaser contents */
    node = root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();

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
        else if (tag.tagName() == KXMLQLCFunctionStep)
        {
            quint32 fid = -1;
            int num = 0;

            num = tag.attribute(KXMLQLCFunctionNumber).toInt();
            fid = tag.text().toUInt();

            /* Don't check for the member function's existence,
               because it might not have been loaded yet. */
            if (num >= m_steps.size())
                m_steps.append(fid);
            else
                m_steps.insert(num, fid);
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

    QMutableListIterator <quint32> it(m_steps);
    while (it.hasNext() == true)
    {
        Function* function = doc()->function(it.next());
        if (function == NULL || function->type() != Function::Scene)
            it.remove();
    }
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void Chaser::preRun(MasterTimer* timer)
{
    m_runner = new ChaserRunner(doc(), stepFunctions(), fadeInSpeed(), fadeOutSpeed(),
                                duration(), direction(), runOrder(), intensity());
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
