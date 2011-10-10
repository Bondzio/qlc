/*
  Q Light Controller
  rgbmatrix.h

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

#ifndef RGBMATRIX_H
#define RGBMATRIX_H

#include <QVector>
#include <QColor>
#include <QSize>
#include <QPair>
#include <QMap>

#include "function.h"

typedef QVector<QVector<QColor> > RGBMap;

class RGBMatrix : public Function
{
    Q_OBJECT
    Q_DISABLE_COPY(RGBMatrix)

   /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    RGBMatrix(Doc* parent);
    ~RGBMatrix();

    /*********************************************************************
     * Copying
     *********************************************************************/
public:
    /** @reimpl */
    virtual Function* createCopy(Doc* doc);

    /** @reimpl */
    virtual bool copyFrom(const Function* function);

    /************************************************************************
     * Fixture Group
     ************************************************************************/
public:
    void setFixtureGroup(quint32 id);
    quint32 fixtureGroup() const;

private:
    quint32 m_fixtureGroup;

    /************************************************************************
     * Pattern
     ************************************************************************/
public:
    enum Pattern {
        OutwardBox,
        FullRows,
        FullColumns
    };

    static QStringList patternNames();

    void setPattern(const Pattern& pat);
    Pattern pattern() const;

    RGBMap colorMap(quint32 step, quint32 totalSteps) const;

    static Pattern stringToPattern(const QString& str);
    static QString patternToString(RGBMatrix::Pattern pat);

private:
    static void outwardBox(qreal step, qreal totalSteps, Function::Direction direction,
                           const QSize& size, const QColor& color, RGBMap& map);
    static void fullRows(qreal step, qreal totalSteps, Function::Direction direction,
                         const QSize& size, const QColor& color, RGBMap& map);
    static void fullColumns(qreal step, qreal totalSteps, Function::Direction direction,
                            const QSize& size, const QColor& color, RGBMap& map);

private:
    Pattern m_pattern;

    /************************************************************************
     * Colour
     ************************************************************************/
public:
    void setMonoColor(const QColor& c);
    QColor monoColor() const;

private:
    QColor m_monoColor;

    /************************************************************************
     * Fade bus
     ************************************************************************/
public:
    void setFadeBus(quint32 id);
    quint32 fadeBus() const;

private:
    quint32 m_fadeBus;

    /************************************************************************
     * Load & Save
     ************************************************************************/
public:
    /** @reimpl */
    bool loadXML(const QDomElement* root);

    /** @reimpl */
    bool saveXML(QDomDocument* doc, QDomElement* root);

    /************************************************************************
     * Running
     ************************************************************************/
public:
    /** @reimpl */
    void preRun(MasterTimer* timer);

    /** @reimpl */
    void write(MasterTimer* timer, UniverseArray* universes);

    /** @reimpl */
    void postRun(MasterTimer* timer, UniverseArray* universes);

private:
    Function::Direction m_direction;
};

#endif
