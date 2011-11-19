/*
  Q Light Controller
  rgbmatrix.cpp

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

#include <QCoreApplication>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomText>
#include <QDebug>
#include <cmath>
#include <QDir>

#include "fixturegroup.h"
#include "genericfader.h"
#include "fadechannel.h"
#include "rgbmatrix.h"
#include "qlcmacros.h"
#include "doc.h"

#define KXMLQLCRGBMatrixScript "Script"
#define KXMLQLCRGBMatrixMonoColor "MonoColor"
#define KXMLQLCRGBMatrixFixtureGroup "FixtureGroup"

/****************************************************************************
 * Initialization
 ****************************************************************************/

RGBMatrix::RGBMatrix(Doc* doc)
    : Function(doc, Function::RGBMatrix)
    , m_fixtureGroup(FixtureGroup::invalidId())
    , m_monoColor(Qt::red)
    , m_fader(NULL)
    , m_step(0)
{
    setName(tr("New RGB Matrix"));
    setDuration(500);
    m_script = RGBScript::script("Full Columns");
}

RGBMatrix::~RGBMatrix()
{
}

/****************************************************************************
 * Copying
 ****************************************************************************/

Function* RGBMatrix::createCopy(Doc* doc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new RGBMatrix(doc);
    if (copy->copyFrom(this) == false || doc->addFunction(copy) == false)
    {
        delete copy;
        copy = NULL;
    }

    return copy;
}

bool RGBMatrix::copyFrom(const Function* function)
{
    const RGBMatrix* mtx = qobject_cast<const RGBMatrix*> (function);
    if (mtx == NULL)
        return false;

    setFixtureGroup(mtx->fixtureGroup());
    setScript(mtx->script());
    setMonoColor(mtx->monoColor());

    return Function::copyFrom(function);
}

/****************************************************************************
 * Fixtures
 ****************************************************************************/

void RGBMatrix::setFixtureGroup(quint32 id)
{
    m_fixtureGroup = id;
}

quint32 RGBMatrix::fixtureGroup() const
{
    return m_fixtureGroup;
}

/****************************************************************************
 * Scripts
 ****************************************************************************/

void RGBMatrix::setScript(const RGBScript& script)
{
    m_script = script;
    m_script.evaluate();
}

RGBScript RGBMatrix::script() const
{
    return m_script;
}

void RGBMatrix::loadScript(const QString& fileName)
{
    QList <QDir> dirs;
#ifndef WIN32
    dirs << RGBScript::userScriptDirectory();
#endif
    dirs << RGBScript::systemScriptDirectory();

    QListIterator <QDir> it(dirs);
    while (it.hasNext() == true)
    {
        QDir dir(it.next());
        if (dir.entryList().contains(fileName) == true)
        {
            if (m_script.load(dir, fileName) == true)
                m_script.evaluate();
            break;
        }
    }
}

QList <RGBMap> RGBMatrix::previewMaps()
{
    QList <RGBMap> steps;

    FixtureGroup* grp = doc()->fixtureGroup(fixtureGroup());
    if (grp != NULL && m_script.apiVersion() > 0)
    {
        for (int i = 0; i < m_script.rgbMapStepCount(grp->size()); i++)
            steps << m_script.rgbMap(grp->size(), monoColor().rgb(), i);
    }

    return steps;
}

/****************************************************************************
 * Colour
 ****************************************************************************/

void RGBMatrix::setMonoColor(const QColor& c)
{
    m_monoColor = c;
}

QColor RGBMatrix::monoColor() const
{
    return m_monoColor;
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool RGBMatrix::loadXML(const QDomElement* root)
{
    QDomNode node;
    QDomElement tag;

    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root->attribute(KXMLQLCFunctionType) != typeToString(Function::RGBMatrix))
    {
        qWarning() << Q_FUNC_INFO << "Function is not an RGB matrix";
        return false;
    }

    /* Load matrix contents */
    node = root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();

        if (tag.tagName() == KXMLQLCFunctionSpeed)
        {
            loadXMLSpeed(tag);
        }
        else if (tag.tagName() == KXMLQLCRGBMatrixScript)
        {
            loadScript(tag.text());
        }
        else if (tag.tagName() == KXMLQLCRGBMatrixFixtureGroup)
        {
            setFixtureGroup(tag.text().toUInt());
        }
        else if (tag.tagName() == KXMLQLCFunctionDirection)
        {
            loadXMLDirection(tag);
        }
        else if (tag.tagName() == KXMLQLCFunctionRunOrder)
        {
            loadXMLRunOrder(tag);
        }
        else if (tag.tagName() == KXMLQLCRGBMatrixMonoColor)
        {
            setMonoColor(QColor::fromRgb(QRgb(tag.text().toUInt())));
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown RGB matrix tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool RGBMatrix::saveXML(QDomDocument* doc, QDomElement* wksp_root)
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

    /* Speeds */
    saveXMLSpeed(doc, &root);

    /* Direction */
    saveXMLDirection(doc, &root);

    /* Run order */
    saveXMLRunOrder(doc, &root);

    /* Script */
    tag = doc->createElement(KXMLQLCRGBMatrixScript);
    root.appendChild(tag);
    text = doc->createTextNode(m_script.fileName());
    tag.appendChild(text);

    /* Mono Color */
    tag = doc->createElement(KXMLQLCRGBMatrixMonoColor);
    root.appendChild(tag);
    text = doc->createTextNode(QString::number(monoColor().rgb()));
    tag.appendChild(text);

    /* Fixture Group */
    tag = doc->createElement(KXMLQLCRGBMatrixFixtureGroup);
    root.appendChild(tag);
    text = doc->createTextNode(QString::number(fixtureGroup()));
    tag.appendChild(text);

    return true;
}

/****************************************************************************
 * Running
 ****************************************************************************/

void RGBMatrix::preRun(MasterTimer* timer)
{
    Q_UNUSED(timer);

    FixtureGroup* grp = doc()->fixtureGroup(fixtureGroup());
    if (grp != NULL)
    {
        m_direction = direction();

        Q_ASSERT(m_fader == NULL);
        m_fader = new GenericFader(doc());

        if (m_direction == Forward)
            m_step = 0;
        else
            m_step = m_script.rgbMapStepCount(grp->size());
    }

    Function::preRun(timer);
}

void RGBMatrix::write(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(timer);
    Q_UNUSED(universes);

    FixtureGroup* grp = doc()->fixtureGroup(fixtureGroup());
    if (grp == NULL)
    {
        // No fixture group to control
        stop();
        return;
    }

    // No time to do anything.
    if (duration() == 0)
        return;

    // Invalid/nonexistent script
    if (m_script.apiVersion() == 0)
        return;

    // Get new map every time when elapsed is reset to zero
    if (elapsed() == 0)
    {
        RGBMap map = m_script.rgbMap(grp->size(), monoColor().rgb(), m_step);
        updateMapChannels(map, grp);
    }

    // Run the generic fader that takes care of fading in/out individual channels
    m_fader->write(universes);

    // Increment elapsed time
    incrementElapsed();

    // Check if we need to change direction, stop completely or go to next step
    if (elapsed() >= duration())
        roundCheck(grp->size());
}

void RGBMatrix::postRun(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(timer);
    Q_UNUSED(universes);

    Q_ASSERT(m_fader != NULL);
    delete m_fader;
    m_fader = NULL;

    Function::postRun(timer, universes);
}

void RGBMatrix::roundCheck(const QSize& size)
{
    if (runOrder() == PingPong)
    {
        if (m_direction == Forward && m_step >= m_script.rgbMapStepCount(size))
        {
            m_direction = Backward;
            m_step = m_script.rgbMapStepCount(size) - 2;
        }
        else if (m_direction == Backward && m_step <= 0)
        {
            m_direction = Forward;
            m_step = 1;
        }
        else
        {
            if (m_direction == Forward)
                m_step++;
            else
                m_step--;
        }
    }
    else if (runOrder() == SingleShot)
    {
        if (m_direction == Forward)
        {
            if (m_step >= m_script.rgbMapStepCount(size) - 1)
                stop();
            else
                m_step++;
        }
    }
    else
    {
        if (m_direction == Forward)
        {
            if (m_step >= m_script.rgbMapStepCount(size) - 1)
                m_step = 0;
            else
                m_step++;
        }
        else
        {
            if (m_step <= 0)
                m_step = m_script.rgbMapStepCount(size) - 1;
            else
                m_step--;
        }
    }

    resetElapsed();
}

void RGBMatrix::updateMapChannels(const RGBMap& map, const FixtureGroup* grp)
{
    // Create/modify fade channels for ALL pixels in the color map.
    for (int y = 0; y < map.size(); y++)
    {
        for (int x = 0; x < map[y].size(); x++)
        {
            QLCPoint pt(x, y);
            Fixture* fxi = doc()->fixture(grp->fixture(pt));
            if (fxi == NULL)
                continue;

            QList <quint32> rgb = fxi->rgbChannels();
            QList <quint32> cmy = fxi->cmyChannels();
            if (rgb.isEmpty() == false)
            {
                // RGB color mixing
                FadeChannel fc;
                fc.setFixture(fxi->id());

                fc.setChannel(rgb.takeFirst());
                fc.setTarget(qRed(map[y][x]));
                insertStartValues(fc);
                m_fader->add(fc);

                fc.setChannel(rgb.takeFirst());
                fc.setTarget(qGreen(map[y][x]));
                insertStartValues(fc);
                m_fader->add(fc);

                fc.setChannel(rgb.takeFirst());
                fc.setTarget(qBlue(map[y][x]));
                insertStartValues(fc);
                m_fader->add(fc);
            }
            else if (cmy.isEmpty() == false)
            {
                // CMY color mixing
                QColor col(map[y][x]);

                FadeChannel fc;
                fc.setFixture(fxi->id());

                fc.setChannel(cmy.takeFirst());
                fc.setTarget(col.cyan());
                insertStartValues(fc);
                m_fader->add(fc);

                fc.setChannel(cmy.takeFirst());
                fc.setTarget(col.magenta());
                insertStartValues(fc);
                m_fader->add(fc);

                fc.setChannel(cmy.takeFirst());
                fc.setTarget(col.yellow());
                insertStartValues(fc);
                m_fader->add(fc);
            }
            else if (fxi->masterIntensityChannel() != QLCChannel::invalid())
            {
                // Simple intensity (dimmer) channel
                QColor col(map[y][x]);
                FadeChannel fc;
                fc.setFixture(fxi->id());
                fc.setChannel(fxi->masterIntensityChannel());
                fc.setTarget(col.value());
                insertStartValues(fc);
                m_fader->add(fc);
            }
        }
    }
}

void RGBMatrix::insertStartValues(FadeChannel& fc) const
{
    Q_ASSERT(m_fader != NULL);

    // To create a nice and smooth fade, get the starting value from
    // m_fader's existing FadeChannel (if any). Otherwise just assume
    // we're starting from zero.
    if (m_fader->channels().contains(fc) == true)
    {
        FadeChannel old = m_fader->channels()[fc];
        fc.setCurrent(old.current());
        fc.setStart(old.current());
    }
    else
    {
        fc.setCurrent(0);
        fc.setStart(0);
    }

    // The channel is not ready yet
    fc.setReady(false);

    // Fade in speed is used for all non-zero targets
    if (fc.target() == 0)
        fc.setFadeTime(fadeOutSpeed());
    else
        fc.setFadeTime(fadeInSpeed());
}
