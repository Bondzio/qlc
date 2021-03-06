/*
  Q Light Controller
  efx.h

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

#ifndef EFX_H
#define EFX_H

#include <QVector>
#include <QPoint>
#include <QList>

#include "efxfixture.h"
#include "function.h"

class QDomDocument;
class QDomElement;
class GenericFader;
class QString;
class Fixture;

#define KXMLQLCEFXPropagationMode "PropagationMode"
#define KXMLQLCEFXPropagationModeParallel "Parallel"
#define KXMLQLCEFXPropagationModeSerial "Serial"
#define KXMLQLCEFXPropagationModeAsymmetric "Asymmetric"
#define KXMLQLCEFXAlgorithm "Algorithm"
#define KXMLQLCEFXWidth "Width"
#define KXMLQLCEFXHeight "Height"
#define KXMLQLCEFXRotation "Rotation"
#define KXMLQLCEFXAxis "Axis"
#define KXMLQLCEFXOffset "Offset"
#define KXMLQLCEFXFrequency "Frequency"
#define KXMLQLCEFXPhase "Phase"
#define KXMLQLCEFXChannel "Channel"
#define KXMLQLCEFXX "X"
#define KXMLQLCEFXY "Y"
#define KXMLQLCEFXStartScene "StartScene"
#define KXMLQLCEFXStopScene "StopScene"

#define KXMLQLCEFXCircleAlgorithmName "Circle"
#define KXMLQLCEFXEightAlgorithmName "Eight"
#define KXMLQLCEFXLineAlgorithmName "Line"
#define KXMLQLCEFXDiamondAlgorithmName "Diamond"
#define KXMLQLCEFXLissajousAlgorithmName "Lissajous"

/**
 * An EFX (effects) function that is used to create
 * more complex automation especially for moving lights
 */
class EFX : public Function
{
    Q_OBJECT
    Q_DISABLE_COPY(EFX)

    friend class EFXFixture;

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    EFX(Doc* doc);
    ~EFX();

    /*********************************************************************
     * Copying
     *********************************************************************/
public:
    /** @reimpl */
    Function* createCopy(Doc* doc);

    /** Copy the contents for this function from another function */
    bool copyFrom(const Function* function);

    /*********************************************************************
     * Algorithm
     *********************************************************************/
public:
    enum Algorithm
    {
        Circle,
        Eight,
        Line,
        Diamond,
        Lissajous
    };

    /** Get the current algorithm */
    Algorithm algorithm() const;

    /** Set the current algorithm */
    void setAlgorithm(Algorithm algo);

    /** Get the supported algorithms in a string list */
    static QStringList algorithmList();

    /** Convert an algorithm type to a string */
    static QString algorithmToString(Algorithm algo);

    /** Convert a string to an algorithm type */
    static Algorithm stringToAlgorithm(const QString& str);

    /**
     * Get a preview of the current algorithm. Puts 128 points to the
     * given polygon, 255px wide and 255px high at maximum, that represent
     * roughly the path of the pattern on a flat surface directly in front
     * of a moving (head/mirror) fixture.
     *
     * @param direction Forward or Backward
     * @param polygon The polygon to fill with preview points
     */
    bool preview(Function::Direction direction, QVector <QPoint>& polygon) const;

private:
    /**
     * Calculate a single point with the currently selected algorithm,
     * based on the value of iterator (which is basically a step number).
     *
     * @param direction Forward or Backward (input)
     * @param iterator Step number (input)
     * @param x Used to store the calculated X coordinate (output)
     * @param y Used to store the calculated Y coordinate (output)
     */
    void calculatePoint(Function::Direction direction, qreal iterator, qreal* x, qreal* y) const;

    /**
     * Rotate a point of the pattern by rot degrees and scale the point
     * within w/h and xOff/yOff.
     *
     * @param x Holds the calculated X coordinate
     * @param y Holds the calculated Y coordinate
     * @param w The width to scale to
     * @param h The height to scale to
     * @param xOff X offset of the pattern
     * @param yOff Y offset of the pattern
     * @param rotation Degrees to rotate
     */
    void rotateAndScale(qreal *x, qreal *y) const;

    /**
     * Calculate a single point with the currently selected algorithm,
     * based on the value of iterator (which is basically a step number).
     *
     * @param iterator Step number (input)
     * @param x Used to store the calculated X coordinate (output)
     * @param y Used to store the calculated Y coordinate (output)
     */
    void calculatePoint(qreal iterator, qreal* x, qreal* y) const;

    /**
     * Recalculate iterator depending on direction
     *
     * @param direction Forward or Backward
     * @param iterator Step number (input)
     */
    qreal calculateDirection(Function::Direction direction, qreal iterator) const;

private:
    /** Current algorithm used by the EFX */
    Algorithm m_algorithm;

    /*********************************************************************
     * Width
     *********************************************************************/
public:
    /**
     * Set the pattern width
     *
     * @param width Pattern width (0-255)
     */
    void setWidth(int width);

    /**
     * Get the pattern width
     *
     * @return Pattern width (0-255)
     */
    int width() const;

private:
    /**
     * Pattern width, see setWidth()
     */
    qreal m_width;

    /*********************************************************************
     * Height
     *********************************************************************/
public:
    /**
     * Set the pattern height
     *
     * @param height Pattern height (0-255)
     */
    void setHeight(int height);

    /**
     * Get the pattern height
     *
     * @return Pattern height (0-255)
     */
    int height() const;

private:
    /**
     * Pattern height, see setHeight()
     */
    qreal m_height;

    /*********************************************************************
     * Rotation
     *********************************************************************/
public:
    /**
     * Set the pattern rotation
     *
     * @param rot Pattern rotation (0-359)
     */
    void setRotation(int rot);

    /**
     * Get the pattern rotation
     *
     * @return Pattern rotation (0-359)
     */
    int rotation() const;

private:
    /**
     * Update m_cosR and m_sinR after m_rotation change
     */
    void updateRotationCache();

private:
    /**
     * Pattern rotation, see setRotation()
     */
    int m_rotation;

    /**
     * cached cos(m_rotation) to speed up computation
     */
    qreal m_cosR;

    /**
     * cached sin(m_rotation) to speed up computation
     */
    qreal m_sinR;

    /*********************************************************************
     * Offset
     *********************************************************************/
public:
    /**
     * Set the pattern offset on the X-axis
     *
     * @param offset Pattern offset (0-255; 127 is middle)
     */
    void setXOffset(int offset);

    /**
     * Get the pattern offset on the X-axis
     *
     * @return Pattern offset (0-255; 127 is middle)
     */
    int xOffset() const;

    /**
     * Set the pattern offset on the Y-axis
     *
     * @param offset Pattern offset (0-255; 127 is middle)
     */
    void setYOffset(int offset);

    /**
     * Get the pattern offset on the Y-axis
     *
     * @return Pattern offset (0-255; 127 is middle)
     */
    int yOffset() const;

private:
    /**
     * Pattern X offset, see setXOffset()
     */
    qreal m_xOffset;

    /**
     * Pattern Y offset, see setXOffset()
     */
    qreal m_yOffset;

    /*********************************************************************
     * Frequency
     *********************************************************************/
public:
    /**
     * Set the lissajous pattern frequency on the X-axis
     *
     * @param freq Pattern frequency (0-5)
     */
    void setXFrequency(int freq);

    /**
     * Get the lissajous pattern frequency on the X-axis
     *
     * @return Pattern frequency (0-5)
     */
    int xFrequency() const;

    /**
     * Set the lissajous pattern frequency on the Y-axis
     *
     * @param freq Pattern frequency (0-5)
     */
    void setYFrequency(int freq);

    /**
     * Get the lissajous pattern frequency on the Y-axis
     *
     * @return Pattern frequency (0-5)
     */
    int yFrequency() const;

    /**
     * Returns true when lissajous has been selected
     */
    bool isFrequencyEnabled();

private:
    /**
     * Lissajous pattern X frequency, see setXFrequency()
     */
    qreal m_xFrequency;

    /**
     * Lissajous pattern Y frequency, see setYFrequency()
     */
    qreal m_yFrequency;

    /*********************************************************************
     * Phase
     *********************************************************************/
public:
    /**
     * Set the lissajous pattern phase on the X-axis
     *
     * @param phase Pattern phase (0-359)
     */
    void setXPhase(int phase);

    /**
     * Get the lissajous pattern phase on the X-axis
     *
     * @return Pattern phase (0-359)
     */
    int xPhase() const;

    /**
     * Set the lissajous pattern phase on the Y-axis
     *
     * @param phase Pattern phase (0-359)
     */
    void setYPhase(int phase);

    /**
     * Get the lissajous pattern phase on the Y-axis
     *
     * @return Pattern phase (0-359)
     */
    int yPhase() const;

    /**
     * Returns true when lissajous has been selected
     */
    bool isPhaseEnabled() const;

private:
    /**
     * Lissajous pattern X phase, see setXPhase()
     */
    qreal m_xPhase;

    /**
     * Lissajous pattern Y phase, see setYPhase()
     */
    qreal m_yPhase;

    /*********************************************************************
     * Fixtures
     *********************************************************************/
public:
    /** Add a new fixture to this EFX */
    bool addFixture(EFXFixture* ef);

    /** Remove the designated fixture from this EFX but don't delete it */
    bool removeFixture(EFXFixture* ef);

    /** Raise a fixture in the serial order to an earlier position */
    bool raiseFixture(EFXFixture* ef);

    /** Lower a fixture in the serial order to a later position */
    bool lowerFixture(EFXFixture* ef);

    /** Get a list of fixtures taking part in this EFX */
    const QList <EFXFixture*> fixtures() const;

public slots:
    /** Slot that captures Doc::fixtureRemoved signals */
    void slotFixtureRemoved(quint32 fxi_id);

private:
    QList <EFXFixture*> m_fixtures;
    GenericFader* m_fader;

    /*********************************************************************
     * Fixture propagation mode
     *********************************************************************/
public:
    enum PropagationMode
    {
        Parallel, /**< All fixtures move in unison (el-cheapo) */
        Serial, /**< Pattern propagates to the next fixture after a delay */
        Asymmetric /**< All fixtures move with an offset */
    };

    /** Set the EFX's fixture propagation mode (see the enum above) */
    void setPropagationMode(PropagationMode mode);

    /** Get the EFX's fixture propagation mode */
    PropagationMode propagationMode() const;

    /** Convert the propagation mode setting to a string */
    static QString propagationModeToString(PropagationMode mode);

    /** Convert a string to a propagation mode setting */
    static PropagationMode stringToPropagationMode(QString str);

private:
    PropagationMode m_propagationMode;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root);
    bool loadXML(const QDomElement& root);
    void postLoad();

private:
    /** Load an axis' contents from an XML document*/
    bool loadXMLAxis(const QDomElement& root);

    /*********************************************************************
     * Speed
     *********************************************************************/
private:
    quint32 m_legacyFadeBus;
    quint32 m_legacyHoldBus;

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

    /*********************************************************************
     * Intensity
     *********************************************************************/
public:
    /** @reimp */
    void adjustIntensity(qreal fraction);
};

#endif
