/*
  Q Light Controller
  scene.cpp

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

#include <QtDebug>
#include <QList>
#include <QFile>
#include <QtXml>

#include "qlcfixturedef.h"
#include "qlcfile.h"

#include "universearray.h"
#include "mastertimer.h"
#include "scene.h"
#include "doc.h"
#include "bus.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Scene::Scene(Doc* doc) : Function(doc)
{
    setName(tr("New Scene"));
    setBus(Bus::defaultFade());
}

Scene::~Scene()
{
}

/*****************************************************************************
 * Function type
 *****************************************************************************/

Function::Type Scene::type() const
{
    return Function::Scene;
}

/*****************************************************************************
 * Copying
 *****************************************************************************/

Function* Scene::createCopy(Doc* doc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new Scene(doc);
    Q_ASSERT(copy != NULL);

    if (copy->copyFrom(this) == false)
    {
        delete copy;
        copy = NULL;
    }
    else if (doc->addFunction(copy) == false)
    {
        delete copy;
        copy = NULL;
    }
    else
    {
        copy->setName(tr("Copy of %1").arg(name()));
    }

    return copy;
}

bool Scene::copyFrom(const Function* function)
{
    const Scene* scene = qobject_cast<const Scene*> (function);
    if (scene == NULL)
        return false;

    m_values.clear();
    m_values = scene->m_values;

    bool result = Function::copyFrom(function);

    emit changed(m_id);

    return result;
}

/*****************************************************************************
 * Values
 *****************************************************************************/

void Scene::setValue(const SceneValue& scv)
{
    int index = m_values.indexOf(scv);
    if (index == -1)
        m_values.append(scv);
    else
        m_values.replace(index, scv);

    emit changed(m_id);
}

void Scene::setValue(t_fixture_id fxi, quint32 ch, uchar value)
{
    setValue(SceneValue(fxi, ch, value));
}

void Scene::unsetValue(t_fixture_id fxi, quint32 ch)
{
    m_values.removeAll(SceneValue(fxi, ch, 0));
    emit changed(m_id);
}

uchar Scene::value(t_fixture_id fxi, quint32 ch)
{
    SceneValue scv(fxi, ch, 0);
    int index = m_values.indexOf(scv);
    if (index == -1)
        return 0;
    else
        return m_values.at(index).value;
}

QList <SceneValue> Scene::values() const
{
    return m_values;
}

void Scene::clear()
{
    m_values.clear();
}

/*****************************************************************************
 * Fixtures
 *****************************************************************************/

void Scene::slotFixtureRemoved(t_fixture_id fxi_id)
{
    QMutableListIterator <SceneValue> it(m_values);
    while (it.hasNext() == true)
    {
        SceneValue scv = it.next();
        if (scv.fxi == fxi_id)
            it.remove();
    }

    emit changed(m_id);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool Scene::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement root;
    QDomElement tag;
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

    /* Speed bus */
    tag = doc->createElement(KXMLQLCBus);
    root.appendChild(tag);
    tag.setAttribute(KXMLQLCBusRole, KXMLQLCBusFade);
    str.setNum(busID());
    text = doc->createTextNode(str);
    tag.appendChild(text);

    /* Scene contents */
    QListIterator <SceneValue> it(m_values);
    while (it.hasNext() == true)
        it.next().saveXML(doc, &root);

    return true;
}

bool Scene::loadXML(const QDomElement* root)
{
    QString str;

    QDomNode node;
    QDomElement tag;

    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root->attribute(KXMLQLCFunctionType) !=
            typeToString(Function::Scene))
    {
        qWarning() << Q_FUNC_INFO << "Function is not a scene";
        return false;
    }

    /* Load scene contents */
    node = root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();

        if (tag.tagName() == KXMLQLCBus)
        {
            /* Bus */
            setBus(tag.text().toUInt());
        }
        else if (tag.tagName() == KXMLQLCFunctionValue)
        {
            /* Channel value */
            SceneValue scv;
            if (scv.loadXML(tag) == true)
                setValue(scv);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown scene tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

/****************************************************************************
 * Flashing
 ****************************************************************************/

void Scene::flash(MasterTimer* timer)
{
    if (flashing() == true)
        return;

    Q_ASSERT(timer != NULL);
    Function::flash(timer);
    timer->registerDMXSource(this);
}

void Scene::unFlash(MasterTimer* timer)
{
    if (flashing() == false)
        return;

    Q_ASSERT(timer != NULL);
    Function::unFlash(timer);
}

void Scene::writeDMX(MasterTimer* timer, UniverseArray* universes)
{
    Q_ASSERT(timer != NULL);
    Q_ASSERT(universes != NULL);

    if (flashing() == true)
        writeValues(universes);
    else
        timer->unregisterDMXSource(this);
}

/****************************************************************************
 * Running
 ****************************************************************************/

void Scene::arm()
{
    m_armedChannels.clear();

    /* Scenes cannot run unless they are children of Doc */
    Doc* doc = qobject_cast <Doc*> (parent());
    Q_ASSERT(doc != NULL);

    /* Get exact address numbers from fixtures and fixate them to this
       scene for running. */
    QMutableListIterator <SceneValue> it(m_values);
    while (it.hasNext() == true)
    {
        SceneValue scv(it.next());

        Fixture* fxi = doc->fixture(scv.fxi);
        if (fxi == NULL)
        {
            qWarning() << Q_FUNC_INFO << "Channel" << scv.channel << "from an"
                       << "unavailable fixture ID" << scv.fxi
                       << "taking part in scene" << name()
                       << ". Removing the channel.";
            it.remove();
            continue;
        }

        if (scv.channel < fxi->channels())
        {
            const QLCChannel* qlcch = fxi->channel(scv.channel);
            Q_ASSERT(qlcch != NULL);

            FadeChannel fc;
            fc.setAddress(fxi->universeAddress() + scv.channel);
            fc.setGroup(qlcch->group());
            fc.setTarget(scv.value);
            m_armedChannels << fc;
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Scene " << name() << "channel"
                       << scv.channel << "is out of its fixture" << fxi->name()
                       << "channel count ( <" << fxi->channels()
                       << ") bounds. Removing the channel.";
            it.remove();
        }
    }

    resetElapsed();
}

void Scene::disarm()
{
    m_armedChannels.clear();
}

void Scene::write(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(timer);
    Q_ASSERT(universes != NULL);

    /* Count ready channels so that the scene can be stopped */
    quint32 ready = m_armedChannels.count();

    /* Iterator for all scene channels */
    QMutableListIterator <FadeChannel> it(m_armedChannels);

    /* Get starting values for each channel on the first pass */
    if (elapsed() == 0)
    {
        while (it.hasNext() == true)
        {
            FadeChannel& fc(it.next());

            /* Get the starting value from universes. Important
               to cast to uchar, since UniverseArray handles signed
               char, whereas uchar is unsigned. Without cast,
               this will result in negative values when x > 127 */
            fc.setStart(uchar(universes->preGMValues()[fc.address()]));
            fc.setCurrent(fc.start());
        }

        /* Reel back to start of list */
        it.toFront();
    }

    // Grab current fade bus value
    quint32 fadeTime = Bus::instance()->value(m_busID);

    while (it.hasNext() == true)
    {
        FadeChannel& fc(it.next());
        if (fc.current() == fc.target())
        {
            /* Write the target value to the universe */
            universes->write(fc.address(), fc.target(), fc.group());
            ready--;
            continue;
        }

        if (elapsed() >= fadeTime)
        {
            /* When this scene's time is up, write the absolute
               target values to get rid of rounding errors that
               may happen in nextValue(). */
            fc.setCurrent(fc.target());
            ready--;

            /* Write the target value to the universe */
            universes->write(fc.address(), fc.target(), fc.group());
        }
        else
        {
            /* Write the next value to the universe buffer */
            universes->write(fc.address(),
                             fc.calculateCurrent(fadeTime, elapsed()),
                             fc.group());
        }
    }

    /* Next time unit */
    incrementElapsed();

    /* When all channels are ready, this function can be stopped. */
    if (ready == 0)
        stop();
}

void Scene::writeValues(UniverseArray* universes, t_fixture_id fxi_id)
{
    Q_ASSERT(universes != NULL);

    for (int i = 0; i < m_values.size(); i++)
    {
        if (fxi_id == Fixture::invalidId() || m_values[i].fxi == fxi_id)
        {
            const FadeChannel& fc(m_armedChannels[i]);
            universes->write(fc.address(), fc.target(), fc.group());
        }
    }
}

void Scene::writeZeros(UniverseArray* universes, t_fixture_id fxi_id)
{
    Q_ASSERT(universes != NULL);

    for (int i = 0; i < m_values.size(); i++)
    {
        if (fxi_id == Fixture::invalidId() || m_values[i].fxi == fxi_id)
        {
            const FadeChannel& fc(m_armedChannels[i]);
            universes->write(fc.address(), 0, fc.group());
        }
    }
}

QList <FadeChannel> Scene::armedChannels() const
{
    return m_armedChannels;
}

