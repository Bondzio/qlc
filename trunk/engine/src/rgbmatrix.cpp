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

#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomText>
#include <QDebug>
#include <cmath>

#include "fixturegroup.h"
#include "genericfader.h"
#include "fadechannel.h"
#include "rgbmatrix.h"
#include "qlcmacros.h"
#include "doc.h"

#define KXMLQLCRGBMatrixPattern "Pattern"
#define KXMLQLCRGBMatrixMonoColor "MonoColor"
#define KXMLQLCRGBMatrixFixtureGroup "FixtureGroup"

#define KPatternOutwardBox  "Outward Box"
#define KPatternFullRows    "Full Rows"
#define KPatternFullColumns "Full Columns"
#define KPatternEvenOddRows "Even/Odd Rows"

/****************************************************************************
 * Initialization
 ****************************************************************************/

RGBMatrix::RGBMatrix(Doc* doc)
    : Function(doc, Function::RGBMatrix)
    , m_fixtureGroup(FixtureGroup::invalidId())
    , m_pattern(RGBMatrix::OutwardBox)
    , m_stepH(-1)
    , m_stepW(-1)
    , m_monoColor(Qt::red)
    , m_fader(NULL)
{
    setName(tr("New RGB Matrix"));
    setDuration(5 * 1000);
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

    m_fixtureGroup = mtx->m_fixtureGroup;
    m_pattern = mtx->m_pattern;
    m_monoColor = mtx->m_monoColor;

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
 * Pattern
 ****************************************************************************/

QStringList RGBMatrix::patternNames()
{
    QStringList list;
    list << patternToString(RGBMatrix::OutwardBox);
    list << patternToString(RGBMatrix::FullRows);
    list << patternToString(RGBMatrix::FullColumns);
    list << patternToString(RGBMatrix::EvenOddRows);
    return list;
}

void RGBMatrix::setPattern(const RGBMatrix::Pattern& pat)
{
    m_pattern = pat;
    m_stepH = -1;
    m_stepW = -1;
}

RGBMatrix::Pattern RGBMatrix::pattern() const
{
    return m_pattern;
}

RGBMap RGBMatrix::colorMap(uint elapsed, uint duration, bool* changed)
{
    FixtureGroup* grp = doc()->fixtureGroup(fixtureGroup());
    if (grp == NULL)
        return RGBMap();

    if (m_colorMap.size() != grp->size().height() ||
        m_colorMap[0].size() != grp->size().width())
    {
        m_colorMap = RGBMap(grp->size().height());
        for (int y = 0; y < m_colorMap.size(); y++)
            m_colorMap[y].fill(0, grp->size().width());
    }

    bool colorMapChanged = false;
    switch (pattern())
    {
    case OutwardBox:
        colorMapChanged = outwardBox(elapsed, duration, direction(), grp->size(),
                                     monoColor().rgba(), m_colorMap);
        break;
    case FullRows:
        colorMapChanged = fullRows(elapsed, duration, direction(), grp->size(),
                                   monoColor().rgba(), m_colorMap);
        break;
    case FullColumns:
        colorMapChanged = fullColumns(elapsed, duration, direction(), grp->size(),
                                      monoColor().rgba(), m_colorMap);
        break;
    case EvenOddRows:
        colorMapChanged = evenOddRows(elapsed, duration, direction(), grp->size(),
                                      monoColor().rgba(), m_colorMap);
        break;
    default:
        break;
    }

    if (changed != NULL)
        *changed = colorMapChanged;

    return m_colorMap;
}

RGBMatrix::Pattern RGBMatrix::stringToPattern(const QString& str)
{
    if (str == KPatternOutwardBox)
        return OutwardBox;
    else if (str == KPatternFullRows)
        return FullRows;
    else if (str == KPatternFullColumns)
        return FullColumns;
    else if (str == KPatternEvenOddRows)
        return EvenOddRows;
    else
        return OutwardBox;
}

QString RGBMatrix::patternToString(RGBMatrix::Pattern pat)
{
    switch (pat)
    {
    case RGBMatrix::OutwardBox:
        return KPatternOutwardBox;
    case RGBMatrix::FullRows:
        return KPatternFullRows;
    case RGBMatrix::FullColumns:
        return KPatternFullColumns;
    case RGBMatrix::EvenOddRows:
        return KPatternEvenOddRows;
    default:
        return KPatternOutwardBox;
    }
}

bool RGBMatrix::outwardBox(uint elapsed, uint duration, Function::Direction direction,
                           const QSize& size, QRgb color, RGBMap& map)
{
    if (direction == Backward)
        elapsed = duration - elapsed;

    // Make sure we don't go beyond $map's boundaries
    uint hsteps = floor((qreal(size.width()) / qreal(2)) + 0.5);
    uint hstep  = elapsed / (duration / hsteps);
    uint left   = MIN(hsteps - hstep - 1, uint(size.width() - 1));
    uint right  = MIN((size.width() - hsteps) + hstep, uint(size.width() - 1));

    uint vsteps = floor((qreal(size.height()) / qreal(2)) + 0.5);
    uint vstep  = elapsed / (duration / vsteps);
    uint top    = MIN(vsteps - vstep - 1, uint(size.height() - 1));
    uint bottom = MIN((size.height() - vsteps) + vstep, uint(size.height() - 1));

    // Check if the new steps are different than the previous ones
    if (m_stepW != left || m_stepH != top)
    {
        // Need to create a new colormap
        for (int y = 0; y < map.size(); y++)
            map[y].fill(0, map[y].size());

        for (uint i = left; i <= right; i++)
        {
            map[top][i] = color;
            map[bottom][i] = color;
        }

        for (uint i = top; i <= bottom; i++)
        {
            map[i][left] = color;
            map[i][right] = color;
        }

        m_stepW = left;
        m_stepH = top;

        return true;
    }
    else
    {
        // No need to create a new colormap. The previous one applies.
        return false;
    }
}

bool RGBMatrix::fullRows(uint elapsed, uint duration, Function::Direction direction,
                         const QSize& size, QRgb color, RGBMap& map)
{
    if (direction == Backward)
        elapsed = duration - elapsed;

    uint top    = MIN(elapsed / (duration / size.height()), uint(size.height() - 1));
    uint bottom = top;
    uint left   = 0;
    uint right  = size.width();

    // Check if the new steps are different than the previous ones
    if (m_stepH != top)
    {
        // Need to create a new colormap
        for (int y = 0; y < map.size(); y++)
            map[y].fill(0, map[y].size());

        for (uint i = left; i < right; i++)
        {
            map[top][i] = color;
            map[bottom][i] = color;
        }

        m_stepH = top;
        return true;
    }
    else
    {
        // No need to create a new colormap. The previous one applies.
        return false;
    }
}

bool RGBMatrix::fullColumns(uint elapsed, uint duration, Function::Direction direction,
                            const QSize& size, QRgb color, RGBMap& map)
{
    if (direction == Backward)
        elapsed = duration - elapsed;

    uint left   = MIN(elapsed / (duration / size.width()), uint(size.width() - 1));
    uint right  = left;
    uint top    = 0;
    uint bottom = size.height();

    // Check if the new steps are different than the previous ones
    if (m_stepW != left)
    {
        // Need to create a new colormap
        for (int y = 0; y < map.size(); y++)
            map[y].fill(0, map[y].size());

        for (uint i = top; i < bottom; i++)
        {
            map[i][left] = color;
            map[i][right] = color;
        }

        m_stepW = left;
        return true;
    }
    else
    {
        // No need to create a new colormap. The previous one applies.
        return false;
    }
}

bool RGBMatrix::evenOddRows(uint elapsed, uint duration, Function::Direction direction,
                            const QSize& size, QRgb color, RGBMap& map)
{
    if (direction == Backward)
        elapsed = duration - elapsed;

    uint left   = MIN(elapsed / (duration / size.width()), uint(size.width() - 1));
    uint right  = MIN(size.width() - 1 - left, uint(size.width() - 1));
    uint top    = 0;
    uint bottom = size.height();

    if (m_stepW != left)
    {
        // Need to create a new colormap
        for (int y = 0; y < map.size(); y++)
            map[y].fill(0, map[y].size());

        for (uint i = top; i < bottom; i++)
        {
            if ((i % 2) == 0)
                map[i][left] = color;
            else
                map[i][right] = color;
        }

        m_stepW = left;
        return true;
    }
    else
    {
        // No need to create a new colormap. The previous one applies.
        return false;
    }
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
        else if (tag.tagName() == KXMLQLCRGBMatrixPattern)
        {
            setPattern(stringToPattern(tag.text()));
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

    /* Pattern */
    tag = doc->createElement(KXMLQLCRGBMatrixPattern);
    root.appendChild(tag);
    text = doc->createTextNode(patternToString(pattern()));
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
    m_direction = direction();

    Q_ASSERT(m_fader == NULL);
    m_fader = new GenericFader(doc());

    Function::preRun(timer);
}

void RGBMatrix::write(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(timer);
    Q_UNUSED(universes);

    FixtureGroup* grp = doc()->fixtureGroup(fixtureGroup());
    if (grp == NULL)
        return;

    // Get the color map for the next step
    bool changed = false;
    RGBMap map = colorMap(elapsed() % duration(), duration(), &changed);

    // If the color map is identical to the one in the previous step (changed == false),
    // we don't need to update fade channels; we're just fading existing channels in/out,
    // while waiting for the next step.
    if (changed == true)
        updateMapChannels(map, grp);

    // Run the generic fader that takes care of fading in/out individual channels
    m_fader->write(universes);

    // Check if we need to change direction or stop completely
    if (elapsed() >= duration())
    {
        if (runOrder() == Function::PingPong)
        {
            if (m_direction == Function::Backward)
                m_direction = Function::Forward;
            else
                m_direction = Function::Backward;
        }
        else if (runOrder() == Function::SingleShot)
        {
            stop();
        }
    }

    incrementElapsed();
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

            QList <quint32> channels = fxi->rgbChannels();
            if (channels.isEmpty() == false)
            {
                FadeChannel fc;
                fc.setFixture(fxi->id());

                fc.setChannel(channels.takeFirst());
                fc.setTarget(qRed(map[y][x]));
                insertStartValues(fc);
                m_fader->add(fc);

                fc.setChannel(channels.takeFirst());
                fc.setTarget(qGreen(map[y][x]));
                insertStartValues(fc);
                m_fader->add(fc);

                fc.setChannel(channels.takeFirst());
                fc.setTarget(qBlue(map[y][x]));
                insertStartValues(fc);
                m_fader->add(fc);
            }
            else
            {
                channels = fxi->cmyChannels();
                if (channels.isEmpty() == true)
                    continue;

                QColor col(map[y][x]);

                FadeChannel fc;
                fc.setFixture(fxi->id());

                fc.setChannel(channels.takeFirst());
                fc.setTarget(col.cyan());
                insertStartValues(fc);
                m_fader->add(fc);

                fc.setChannel(channels.takeFirst());
                fc.setTarget(col.magenta());
                insertStartValues(fc);
                m_fader->add(fc);

                fc.setChannel(channels.takeFirst());
                fc.setTarget(col.yellow());
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
