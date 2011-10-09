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

#include <QTransform>
#include <QDebug>
#include <cmath>

#include "fixturegroup.h"
#include "rgbmatrix.h"
#include "qlcmacros.h"
#include "doc.h"

#define KPatternOutwardBox  "Outward Box"
#define KPatternFullRows    "Full Rows"
#define KPatternFullColumns "Full Columns"

/****************************************************************************
 * Initialization
 ****************************************************************************/

RGBMatrix::RGBMatrix(Doc* doc)
    : Function(doc, Function::RGBMatrix)
    , m_fixtureGroup(FixtureGroup::invalidId())
    , m_pattern(RGBMatrix::OutwardBox)
{
    setName(tr("New RGB Matrix"));
    setBus(Bus::defaultHold());
    setFadeBus(Bus::defaultFade());
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
    m_fadeBus = mtx->m_fadeBus;

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
    return list;
}

void RGBMatrix::setPattern(const RGBMatrix::Pattern& pat)
{
    m_pattern = pat;
}

RGBMatrix::Pattern RGBMatrix::pattern() const
{
    return m_pattern;
}

RGBMap RGBMatrix::colorMap(quint32 step, quint32 totalSteps) const
{
    FixtureGroup* grp = doc()->fixtureGroup(fixtureGroup());
    if (grp == NULL)
        return RGBMap();

    RGBMap map(grp->size().height());
    for (int y = 0; y < map.size(); y++)
        map[y].fill(Qt::black, grp->size().width());

    switch (pattern())
    {
    case OutwardBox:
        outwardBox(step, totalSteps, direction(), grp->size(), map);
        break;
    case FullRows:
        fullRows(step, totalSteps, direction(), grp->size(), map);
        break;
    case FullColumns:
        fullColumns(step, totalSteps, direction(), grp->size(), map);
        break;
    default:
        break;
    }

    return map;
}

RGBMatrix::Pattern RGBMatrix::stringToPattern(const QString& str)
{
    if (str == KPatternOutwardBox)
        return OutwardBox;
    else if (str == KPatternFullRows)
        return FullRows;
    else if (str == KPatternFullColumns)
        return FullColumns;
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
    default:
        return KPatternOutwardBox;
    }
}

void RGBMatrix::outwardBox(qreal step, qreal totalSteps, Function::Direction direction,
                           const QSize& size, RGBMap& map)
{
    qreal scale = 0;
    if (totalSteps > 0)
        scale = step / totalSteps;

    if (direction == Function::Backward)
        scale = 1.0 - scale;

    // Make sure we don't go beyond $map's boundaries
    qreal left   = ceil(qreal(size.width()) / qreal(2));
    left         = left - (left * scale);
    left         = CLAMP(left, 0, size.width() - 1);

    qreal right  = floor(qreal(size.width()) / qreal(2));
    right        = right + ((size.width() - right) * scale);
    right        = CLAMP(right, 0, size.width() - 1);

    qreal top    = ceil(qreal(size.height()) / qreal(2));
    top          = top - (top * scale);
    top          = CLAMP(top, 0, size.height() - 1);

    qreal bottom = floor(qreal(size.height()) / qreal(2));
    bottom       = bottom + ((size.height() - bottom) * scale);
    bottom       = CLAMP(bottom, 0, size.height() - 1);

    for (int i = left; i <= right; i++) {
        map[top][i] = Qt::yellow;
        map[bottom][i] = Qt::yellow;
    }

    for (int i = top; i <= bottom; i++) {
        map[i][left] = Qt::yellow;
        map[i][right] = Qt::yellow;
    }
}

void RGBMatrix::fullRows(qreal step, qreal totalSteps, Function::Direction direction,
                         const QSize& size, RGBMap& map)
{
    qreal scale = 0;
    if (totalSteps > 0)
        scale = step / totalSteps;

    if (direction == Function::Backward)
        scale = 1.0 - scale;

    // Make sure we don't go beyond $map's boundaries
    qreal left   = qreal(0);
    qreal right  = qreal(size.width() - 1);
    qreal top    = MIN(scale * qreal(size.height()), qreal(size.height() - 1));
    qreal bottom = top;

    for (int i = left; i <= right; i++) {
        map[top][i] = Qt::yellow;
        map[bottom][i] = Qt::yellow;
    }
}

void RGBMatrix::fullColumns(qreal step, qreal totalSteps, Function::Direction direction,
                            const QSize& size, RGBMap& map)
{
    qreal scale = 0;
    if (totalSteps > 0)
        scale = step / totalSteps;

    if (direction == Function::Backward)
        scale = 1.0 - scale;

    // Make sure we don't go beyond $map's boundaries
    qreal left   = MIN(scale * qreal(size.width()), qreal(size.width() - 1));
    qreal right  = left;
    qreal top    = qreal(0);
    qreal bottom = qreal(size.height() - 1);

    for (int i = top; i <= bottom; i++) {
        map[i][left] = Qt::yellow;
        map[i][right] = Qt::yellow;
    }
}

/****************************************************************************
 * Fade bus
 ****************************************************************************/

void RGBMatrix::setFadeBus(quint32 id)
{
    m_fadeBus = id;
}

quint32 RGBMatrix::fadeBus() const
{
    return m_fadeBus;
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool RGBMatrix::loadXML(const QDomElement* root)
{
    Q_UNUSED(root);
    return false;
}

bool RGBMatrix::saveXML(QDomDocument* doc, QDomElement* root)
{
    Q_UNUSED(doc);
    Q_UNUSED(root);

    return false;
}

/****************************************************************************
 * Running
 ****************************************************************************/

void RGBMatrix::preRun(MasterTimer* timer)
{
    Q_UNUSED(timer);
    m_direction = direction();
}

void RGBMatrix::write(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(timer);
    Q_UNUSED(universes);

    incrementElapsed();

    switch (pattern())
    {
    case OutwardBox:
        // outwardBox(elapsed(), busValue(), m_direction, size(), map);
        break;
    default:
        break;
    }

    if (elapsed() >= busValue())
    {
        if (runOrder() == Function::PingPong)
        {
            if (m_direction == Function::Backward)
                m_direction = Function::Forward;
            else
                m_direction = Function::Backward;

            resetElapsed();
        }
        else if (runOrder() == Function::SingleShot)
        {
            stop();
        }
        else // if (runOrder() == Function::Loop)
        {
            resetElapsed();
        }
    }
}

void RGBMatrix::postRun(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(timer);
    Q_UNUSED(universes);
}
