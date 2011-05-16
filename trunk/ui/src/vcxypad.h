/*
  Q Light Controller
  vcxypad.h

  Copyright (c) Stefan Krumm, Heikki Junnila

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

#ifndef VCXYPAD_H
#define VCXYPAD_H

#include <QWidget>
#include <QPixmap>
#include <QString>
#include <QMutex>
#include <QList>

#include "vcxypadfixture.h"
#include "dmxsource.h"
#include "vcwidget.h"

class QDomDocument;
class QDomElement;
class QPaintEvent;
class QMouseEvent;
class MasterTimer;
class QByteArray;
class Doc;

#define KXMLQLCVCXYPad "XYPad"
#define KXMLQLCVCXYPadPosition "Position"
#define KXMLQLCVCXYPadPositionX "X"
#define KXMLQLCVCXYPadPositionY "Y"

class VCXYPad : public VCWidget, public DMXSource
{
    Q_OBJECT
    Q_DISABLE_COPY(VCXYPad)

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    VCXYPad(QWidget* parent, Doc* doc, OutputMap* outputMap, InputMap* inputMap, MasterTimer* masterTimer);
    virtual ~VCXYPad();

    /*************************************************************************
     * Clipboard
     *************************************************************************/
public:
    /** @reimp */
    VCWidget* createCopy(VCWidget* parent);

    /** @reimp */
    bool copyFrom(VCWidget* widget);

    /*************************************************************************
     * Properties
     *************************************************************************/
public:
    /** Display a properties dialog */
    void editProperties();

    /*************************************************************************
     * Fixtures
     *************************************************************************/
public:
    /**
     * Append a new fixture to the XY pad's list of controlled fixtures
     *
     * @param fxi The fixture to append
     */
    void appendFixture(const VCXYPadFixture& fxi);

    /**
     * Remove a fixture by its ID from the XY pad's control list
     */
    void removeFixture(quint32 fxi);

    /**
     * Remove all currently controlled fixtures from the XY pad
     */
    void clearFixtures();

    /**
     * Get a list of the pad's currently controlled fixtures
     */
    QList <VCXYPadFixture> fixtures() const;

protected:
    QList <VCXYPadFixture> m_fixtures;

    /*************************************************************************
     * Current position
     *************************************************************************/
public:
    /** Get the pad's current position (i.e. where the point is) */
    QPoint currentXYPosition() const;

    /** Set the pad's current position (i.e. move the point) */
    void setCurrentXYPosition(const QPoint& point);

    /** @reimp from DMXSource */
    void writeDMX(MasterTimer* timer, UniverseArray* universes);

protected:
    QPoint m_currentXYPosition;
    bool m_currentXYPositionChanged;
    QMutex m_currentXYPositionMutex;

    /*************************************************************************
     * QLC mode
     *************************************************************************/
protected slots:
    /** @reimp */
    void slotModeChanged(Doc::Mode mode);

    /*************************************************************************
     * Load & Save
     *************************************************************************/
public:
    /** @reimp */
    bool loadXML(const QDomElement* root);

    /** @reimp */
    bool saveXML(QDomDocument* doc, QDomElement* root);

    /*************************************************************************
     * Event handlers
     *************************************************************************/
protected:
    /** @reimp */
    void paintEvent(QPaintEvent* e);

    /** @reimp */
    void mousePressEvent(QMouseEvent* e);

    /** @reimp */
    void mouseReleaseEvent(QMouseEvent* e);

    /** @reimp */
    void mouseMoveEvent(QMouseEvent* e);

private:
    QPixmap m_xyPosPixmap;
};

#endif
