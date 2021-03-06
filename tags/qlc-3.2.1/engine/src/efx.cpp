/*
  Q Light Controller
  efx.cpp

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

#include <QVector>
#include <QDebug>
#include <QList>
#include <QtXml>

#include <math.h>

#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "genericfader.h"
#include "qlcchannel.h"
#include "qlcmacros.h"
#include "qlcfile.h"

#include "universearray.h"
#include "mastertimer.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"
#include "efx.h"
#include "bus.h"

/* Supported EFX algorithms */

/*****************************************************************************
 * Initialization
 *****************************************************************************/

EFX::EFX(Doc* doc) : Function(doc)
{
    m_width = 127;
    m_height = 127;
    m_xOffset = 127;
    m_yOffset = 127;
    m_rotation = 0;
    
    updateRotationCache();

    m_xFrequency = 2;
    m_yFrequency = 3;
    m_xPhase = M_PI / 2.0;
    m_yPhase = 0;

    m_propagationMode = Parallel;

    m_algorithm = EFX::Circle;

    setName(tr("New EFX"));

    m_fader = NULL;

    /* Set default speed buses */
    setBus(Bus::defaultHold());
    setFadeBus(Bus::defaultFade());
}

EFX::~EFX()
{
    while (m_fixtures.isEmpty() == false)
        delete m_fixtures.takeFirst();
}

/*****************************************************************************
 * Function type
 *****************************************************************************/

Function::Type EFX::type() const
{
    return Function::EFX;
}

/*****************************************************************************
 * Copying
 *****************************************************************************/

Function* EFX::createCopy(Doc* doc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new EFX(doc);
    if (copy->copyFrom(this) == false || doc->addFunction(copy) == false)
    {
        delete copy;
        copy = NULL;
    }

    return copy;
}

bool EFX::copyFrom(const Function* function)
{
    const EFX* efx = qobject_cast<const EFX*> (function);
    if (efx == NULL)
        return false;

    while (m_fixtures.isEmpty() == false)
        delete m_fixtures.takeFirst();

    QListIterator <EFXFixture*> it(efx->m_fixtures);
    while (it.hasNext() == true)
    {
        EFXFixture* ef = new EFXFixture(this);
        ef->copyFrom(it.next());
        m_fixtures.append(ef);
    }

    m_propagationMode = efx->m_propagationMode;

    m_width = efx->m_width;
    m_height = efx->m_height;
    m_xOffset = efx->m_xOffset;
    m_yOffset = efx->m_yOffset;
    m_rotation = efx->m_rotation;

    updateRotationCache();

    m_xFrequency = efx->m_xFrequency;
    m_yFrequency = efx->m_yFrequency;
    m_xPhase = efx->m_xPhase;
    m_yPhase = efx->m_yPhase;

    m_algorithm = efx->m_algorithm;

    m_fadeBus = efx->m_fadeBus;

    return Function::copyFrom(function);
}

/*****************************************************************************
 * Algorithm
 *****************************************************************************/

EFX::Algorithm EFX::algorithm() const
{
    return m_algorithm;
}

void EFX::setAlgorithm(EFX::Algorithm algo)
{
    if (algo >= EFX::Circle && algo <= EFX::Lissajous)
        m_algorithm = algo;
    else
        m_algorithm = EFX::Circle;

    emit changed(m_id);
}

QStringList EFX::algorithmList()
{
    QStringList list;
    list << algorithmToString(EFX::Circle);
    list << algorithmToString(EFX::Eight);
    list << algorithmToString(EFX::Line);
    list << algorithmToString(EFX::Diamond);
    list << algorithmToString(EFX::Lissajous);
    return list;
}

QString EFX::algorithmToString(EFX::Algorithm algo)
{
    switch (algo)
    {
        default:
        case EFX::Circle:
            return QString(KXMLQLCEFXCircleAlgorithmName);
        case EFX::Eight:
            return QString(KXMLQLCEFXEightAlgorithmName);
        case EFX::Line:
            return QString(KXMLQLCEFXLineAlgorithmName);
        case EFX::Diamond:
            return QString(KXMLQLCEFXDiamondAlgorithmName);
        case EFX::Lissajous:
            return QString(KXMLQLCEFXLissajousAlgorithmName);
    }
}

EFX::Algorithm EFX::stringToAlgorithm(const QString& str)
{
    if (str == QString(KXMLQLCEFXEightAlgorithmName))
        return EFX::Eight;
    else if (str == QString(KXMLQLCEFXLineAlgorithmName))
        return EFX::Line;
    else if (str == QString(KXMLQLCEFXDiamondAlgorithmName))
        return EFX::Diamond;
    else if (str == QString(KXMLQLCEFXLissajousAlgorithmName))
        return EFX::Lissajous;
    else
        return EFX::Circle;
}

bool EFX::preview(QVector <QPoint>& polygon) const
{
    bool retval = true;
    int stepCount = 128;
    int step = 0;
    qreal stepSize = (qreal)(1) / ((qreal)(stepCount) / (M_PI * 2.0));

    qreal i = 0;
    qreal x = 0;
    qreal y = 0;

    /* Resize the array to contain stepCount points */
    polygon.resize(stepCount);

    /* Draw a preview of a circle */
    for (step = 0; step < stepCount; step++)
    {
        calculatePoint(i, &x, &y);
        polygon[step] = QPoint(int(x), int(y));
        i += stepSize;
    }

    return retval;
}

void EFX::calculatePoint(qreal iterator, qreal* x, qreal* y) const
{
    switch (algorithm())
    {
    default:
    case Circle:
        *x = cos(iterator + M_PI_2);
        *y = cos(iterator);
        break;

    case Eight:
        *x = cos((iterator * 2) + M_PI_2);
        *y = cos(iterator);
        break;

    case Line:
        *x = sin(iterator);
        *y = sin(iterator);
        break;

    case Diamond:
        *x = pow(cos(iterator - M_PI_2), 3);
        *y = pow(cos(iterator), 3);
        break;

    case Lissajous:
        *x = cos((m_xFrequency * iterator) - m_xPhase);
        *y = cos((m_yFrequency * iterator) - m_yPhase);
        break;
    }

    rotateAndScale(x, y);
}

void EFX::rotateAndScale(qreal* x, qreal* y) const
{
    qreal xx;
    qreal yy;

    xx = *x;
    yy = *y;

    *x = m_xOffset + xx * m_cosR * m_width + yy * m_sinR * m_height;
    *y = m_yOffset + -xx * m_sinR * m_width + yy * m_cosR * m_height;
}

/*****************************************************************************
 * Width
 *****************************************************************************/

void EFX::setWidth(int width)
{
    m_width = static_cast<double> (CLAMP(width, 0, 127));
    emit changed(m_id);
}

int EFX::width() const
{
    return static_cast<int> (m_width);
}

/*****************************************************************************
 * Height
 *****************************************************************************/

void EFX::setHeight(int height)
{
    m_height = static_cast<double> (CLAMP(height, 0, 127));
    emit changed(m_id);
}

int EFX::height() const
{
    return static_cast<int> (m_height);
}

/*****************************************************************************
 * Rotation
 *****************************************************************************/

void EFX::setRotation(int rot)
{
    m_rotation = static_cast<int> (CLAMP(rot, 0, 359));
    updateRotationCache();
    emit changed(m_id);
}

int EFX::rotation() const
{
    return static_cast<int> (m_rotation);
}

void EFX::updateRotationCache()
{
    qreal r = M_PI/180 * m_rotation;
    m_cosR = cos(r);
    m_sinR = sin(r);
}

/*****************************************************************************
 * Offset
 *****************************************************************************/

void EFX::setXOffset(int offset)
{
    m_xOffset = static_cast<double> (CLAMP(offset, 0, UCHAR_MAX));
    emit changed(m_id);
}

int EFX::xOffset() const
{
    return static_cast<int> (m_xOffset);
}

void EFX::setYOffset(int offset)
{
    m_yOffset = static_cast<double> (CLAMP(offset, 0, UCHAR_MAX));
    emit changed(m_id);
}

int EFX::yOffset() const
{
    return static_cast<int> (m_yOffset);
}

/*****************************************************************************
 * Frequency
 *****************************************************************************/

void EFX::setXFrequency(int freq)
{
    m_xFrequency = static_cast<qreal> (CLAMP(freq, 0, 5));
    emit changed(m_id);
}

int EFX::xFrequency() const
{
    return static_cast<int> (m_xFrequency);
}

void EFX::setYFrequency(int freq)
{
    m_yFrequency = static_cast<qreal> (CLAMP(freq, 0, 5));
    emit changed(m_id);
}

int EFX::yFrequency() const
{
    return static_cast<int> (m_yFrequency);
}

bool EFX::isFrequencyEnabled()
{
    if (m_algorithm == EFX::Lissajous)
        return true;
    else
        return false;
}

/*****************************************************************************
 * Phase
 *****************************************************************************/

void EFX::setXPhase(int phase)
{
    m_xPhase = static_cast<qreal> (CLAMP(phase, 0, 359)) * M_PI / 180.0;
    emit changed(m_id);
}

int EFX::xPhase() const
{
    return static_cast<int> (floor((m_xPhase * 180.0 / M_PI) + 0.5));
}

void EFX::setYPhase(int phase)
{
    m_yPhase = static_cast<qreal> (CLAMP(phase, 0, 359)) * M_PI / 180.0;
    emit changed(m_id);
}

int EFX::yPhase() const
{
    return static_cast<int> (floor((m_yPhase * 180.0 / M_PI) + 0.5));
}

bool EFX::isPhaseEnabled() const
{
    if (m_algorithm == EFX::Lissajous)
        return true;
    else
        return false;
}

/*****************************************************************************
 * Fixtures
 *****************************************************************************/

bool EFX::addFixture(EFXFixture* ef)
{
    Q_ASSERT(ef != NULL);

    /* Search for an existing fixture with the same ID to prevent multiple
       entries of the same fixture. */
    QListIterator <EFXFixture*> it(m_fixtures);
    while (it.hasNext() == true)
    {
        /* Found the same fixture. Don't add the new one. */
        if (it.next()->fixture() == ef->fixture())
            return false;
    }

    /* Put the EFXFixture object into our list */
    m_fixtures.append(ef);

    emit changed(m_id);

    return true;
}

bool EFX::removeFixture(EFXFixture* ef)
{
    Q_ASSERT(ef != NULL);

    if (m_fixtures.removeAll(ef) > 0)
    {
        emit changed(m_id);
        return true;
    }
    else
    {
        return false;
    }
}

bool EFX::raiseFixture(EFXFixture* ef)
{
    Q_ASSERT(ef != NULL);

    int index = m_fixtures.indexOf(ef);
    if (index > 0)
    {
        m_fixtures.move(index, index - 1);
        emit changed(m_id);
        return true;
    }
    else
    {
        return false;
    }
}

bool EFX::lowerFixture(EFXFixture* ef)
{
    int index = m_fixtures.indexOf(ef);
    if (index < (m_fixtures.count() - 1))
    {
        m_fixtures.move(index, index + 1);
        emit changed(m_id);
        return true;
    }
    else
    {
        return false;
    }
}

const QList <EFXFixture*> EFX::fixtures() const
{
    return m_fixtures;
}

void EFX::slotFixtureRemoved(quint32 fxi_id)
{
    /* Remove the destroyed fixture from our list */
    QMutableListIterator <EFXFixture*> it(m_fixtures);
    while (it.hasNext() == true)
    {
        it.next();

        if (it.value()->fixture() == fxi_id)
        {
            delete it.value();
            it.remove();
            break;
        }
    }
}

/*****************************************************************************
 * Fixture propagation mode
 *****************************************************************************/

void EFX::setPropagationMode(PropagationMode mode)
{
    m_propagationMode = mode;
    emit changed(m_id);
}

EFX::PropagationMode EFX::propagationMode() const
{
    return m_propagationMode;
}

QString EFX::propagationModeToString(PropagationMode mode)
{
    if (mode == Serial)
        return QString(KXMLQLCEFXPropagationModeSerial);
    else if (mode == Asymmetric)
        return QString(KXMLQLCEFXPropagationModeAsymmetric);
    else
        return QString(KXMLQLCEFXPropagationModeParallel);
}

EFX::PropagationMode EFX::stringToPropagationMode(QString str)
{
    if (str == QString(KXMLQLCEFXPropagationModeSerial))
        return Serial;
    else if (str == QString(KXMLQLCEFXPropagationModeAsymmetric))
        return Asymmetric;
    else
        return Parallel;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool EFX::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement root;
    QDomElement tag;
    QDomElement subtag;
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

    /* Fixtures */
    QListIterator <EFXFixture*> it(m_fixtures);
    while (it.hasNext() == true)
        it.next()->saveXML(doc, &root);

    /* Propagation mode */
    tag = doc->createElement(KXMLQLCEFXPropagationMode);
    root.appendChild(tag);
    text = doc->createTextNode(propagationModeToString(m_propagationMode));
    tag.appendChild(text);

    /* Hold bus */
    tag = doc->createElement(KXMLQLCBus);
    root.appendChild(tag);
    tag.setAttribute(KXMLQLCBusRole, KXMLQLCBusHold);
    str.setNum(busID());
    text = doc->createTextNode(str);
    tag.appendChild(text);

    /* Fade bus */
    tag = doc->createElement(KXMLQLCBus);
    root.appendChild(tag);
    tag.setAttribute(KXMLQLCBusRole, KXMLQLCBusFade);
    str.setNum(fadeBusID());
    text = doc->createTextNode(str);
    tag.appendChild(text);

    /* Direction */
    tag = doc->createElement(KXMLQLCFunctionDirection);
    root.appendChild(tag);
    text = doc->createTextNode(Function::directionToString(m_direction));
    tag.appendChild(text);

    /* Run order */
    tag = doc->createElement(KXMLQLCFunctionRunOrder);
    root.appendChild(tag);
    text = doc->createTextNode(Function::runOrderToString(m_runOrder));
    tag.appendChild(text);

    /* Algorithm */
    tag = doc->createElement(KXMLQLCEFXAlgorithm);
    root.appendChild(tag);
    text = doc->createTextNode(algorithmToString(algorithm()));
    tag.appendChild(text);

    /* Width */
    tag = doc->createElement(KXMLQLCEFXWidth);
    root.appendChild(tag);
    str.setNum(width());
    text = doc->createTextNode(str);
    tag.appendChild(text);

    /* Height */
    tag = doc->createElement(KXMLQLCEFXHeight);
    root.appendChild(tag);
    str.setNum(height());
    text = doc->createTextNode(str);
    tag.appendChild(text);

    /* Rotation */
    tag = doc->createElement(KXMLQLCEFXRotation);
    root.appendChild(tag);
    str.setNum(rotation());
    text = doc->createTextNode(str);
    tag.appendChild(text);

    /********************************************
     * X-Axis
     ********************************************/
    tag = doc->createElement(KXMLQLCEFXAxis);
    root.appendChild(tag);
    tag.setAttribute(KXMLQLCFunctionName, KXMLQLCEFXX);

    /* Offset */
    subtag = doc->createElement(KXMLQLCEFXOffset);
    tag.appendChild(subtag);
    str.setNum(xOffset());
    text = doc->createTextNode(str);
    subtag.appendChild(text);

    /* Frequency */
    subtag = doc->createElement(KXMLQLCEFXFrequency);
    tag.appendChild(subtag);
    str.setNum(xFrequency());
    text = doc->createTextNode(str);
    subtag.appendChild(text);

    /* Phase */
    subtag = doc->createElement(KXMLQLCEFXPhase);
    tag.appendChild(subtag);
    str.setNum(xPhase());
    text = doc->createTextNode(str);
    subtag.appendChild(text);

    /********************************************
     * Y-Axis
     ********************************************/
    tag = doc->createElement(KXMLQLCEFXAxis);
    root.appendChild(tag);
    tag.setAttribute(KXMLQLCFunctionName, KXMLQLCEFXY);

    /* Offset */
    subtag = doc->createElement(KXMLQLCEFXOffset);
    tag.appendChild(subtag);
    str.setNum(yOffset());
    text = doc->createTextNode(str);
    subtag.appendChild(text);

    /* Frequency */
    subtag = doc->createElement(KXMLQLCEFXFrequency);
    tag.appendChild(subtag);
    str.setNum(yFrequency());
    text = doc->createTextNode(str);
    subtag.appendChild(text);

    /* Phase */
    subtag = doc->createElement(KXMLQLCEFXPhase);
    tag.appendChild(subtag);
    str.setNum(yPhase());
    text = doc->createTextNode(str);
    subtag.appendChild(text);

    return true;
}

bool EFX::loadXML(const QDomElement* root)
{
    QString str;
    QDomNode node;
    QDomElement tag;

    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCFunction)
    {
        qWarning() << "Function node not found!";
        return false;
    }

    if (root->attribute(KXMLQLCFunctionType) != typeToString(Function::EFX))
    {
        qWarning("Function is not an EFX!");
        return false;
    }

    /* Load EFX contents */
    node = root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();

        if (tag.tagName() == KXMLQLCBus)
        {
            /* Bus */
            str = tag.attribute(KXMLQLCBusRole);
            if (str == KXMLQLCBusHold)
                setBus(tag.text().toUInt());
            else if (str == KXMLQLCBusFade)
                setFadeBus(tag.text().toUInt());
        }
        else if (tag.tagName() == KXMLQLCEFXFixture)
        {
            EFXFixture* ef = new EFXFixture(this);
            ef->loadXML(&tag);
            if (ef->fixture() != Fixture::invalidId())
            {
                if (addFixture(ef) == false)
                    delete ef;
            }
        }
        else if (tag.tagName() == KXMLQLCEFXPropagationMode)
        {
            /* Propagation mode */
            setPropagationMode(stringToPropagationMode(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCEFXAlgorithm)
        {
            /* Algorithm */
            setAlgorithm(stringToAlgorithm(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCFunctionDirection)
        {
            /* Direction */
            setDirection(Function::stringToDirection(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCFunctionRunOrder)
        {
            /* Run Order */
            setRunOrder(Function::stringToRunOrder(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCEFXWidth)
        {
            /* Width */
            setWidth(tag.text().toInt());
        }
        else if (tag.tagName() == KXMLQLCEFXHeight)
        {
            /* Height */
            setHeight(tag.text().toInt());
        }
        else if (tag.tagName() == KXMLQLCEFXRotation)
        {
            /* Rotation */
            setRotation(tag.text().toInt());
        }
        else if (tag.tagName() == KXMLQLCEFXAxis)
        {
            /* Axes */
            loadXMLAxis(&tag);
        }
        else
        {
            qWarning() << "Unknown EFX tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool EFX::loadXMLAxis(const QDomElement* root)
{
    int frequency = 0;
    int offset = 0;
    int phase = 0;
    QString axis;

    QDomNode node;
    QDomElement tag;

    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCEFXAxis)
    {
        qWarning() << "EFX axis node not found!";
        return false;
    }

    /* Get the axis name */
    axis = root->attribute(KXMLQLCFunctionName);

    /* Load axis contents */
    node = root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();

        if (tag.tagName() == KXMLQLCEFXOffset)
        {
            offset = tag.text().toInt();
        }
        else if (tag.tagName() == KXMLQLCEFXFrequency)
        {
            frequency = tag.text().toInt();
        }
        else if (tag.tagName() == KXMLQLCEFXPhase)
        {
            phase = tag.text().toInt();
        }
        else
        {
            qWarning() << "Unknown EFX axis tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    if (axis == KXMLQLCEFXY)
    {
        setYOffset(offset);
        setYFrequency(frequency);
        setYPhase(phase);

        return true;
    }
    else if (axis == KXMLQLCEFXX)
    {
        setXOffset(offset);
        setXFrequency(frequency);
        setXPhase(phase);

        return true;
    }
    else
    {
        qWarning() << "Unknown EFX axis:" << axis;

        return false;
    }
}

/*****************************************************************************
 * Bus
 *****************************************************************************/

void EFX::setFadeBus(quint32 id)
{
    if (id < Bus::count())
        m_fadeBus = id;
}

quint32 EFX::fadeBusID() const
{
    return m_fadeBus;
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void EFX::arm()
{
    int serialNumber = 0;

    Doc* doc = qobject_cast <Doc*> (parent());
    Q_ASSERT(doc != NULL);

    QListIterator <EFXFixture*> it(m_fixtures);
    while (it.hasNext() == true)
    {
        EFXFixture* ef = it.next();
        Q_ASSERT(ef != NULL);

        ef->setSerialNumber(serialNumber++);

        /* If fxi == NULL, the fixture has been destroyed */
        Fixture* fxi = doc->fixture(ef->fixture());
        if (fxi == NULL)
            continue;

        /* If this fixture has no mode, it's a generic dimmer that
           can't do pan&tilt anyway. */
        const QLCFixtureMode* mode = fxi->fixtureMode();
        if (mode == NULL)
            continue;

        QList <quint32> intensityChannels;

        /* Find exact channel numbers for MSB/LSB pan and tilt */
        for (quint32 i = 0; i < quint32(mode->channels().size()); i++)
        {
            QLCChannel* ch = mode->channel(i);
            Q_ASSERT(ch != NULL);

            if (ch->group() == QLCChannel::Pan)
            {
                if (ch->controlByte() == QLCChannel::MSB)
                    ef->setMsbPanChannel(fxi->universeAddress() + i);
                else if (ch->controlByte() == QLCChannel::LSB)
                    ef->setLsbPanChannel(fxi->universeAddress() + i);
            }
            else if (ch->group() == QLCChannel::Tilt)
            {
                if (ch->controlByte() == QLCChannel::MSB)
                    ef->setMsbTiltChannel(fxi->universeAddress() + i);
                else if (ch->controlByte() == QLCChannel::LSB)
                    ef->setLsbTiltChannel(fxi->universeAddress() + i);
            }
            else if (ch->group() == QLCChannel::Intensity &&
                     ch->colour() == QLCChannel::NoColour) // Don't touch RGB/CMY channels
            {
                if (ch->searchCapability(/*D*/"immer", false) != NULL ||
                    ch->searchCapability(/*I*/"ntensity", false) != NULL)
                {
                    intensityChannels << (fxi->universeAddress() + i);
                }
            }
        }

        ef->setIntensityChannels(intensityChannels);
        ef->setFadeBus(fadeBusID());
    }

    Q_ASSERT(m_fader == NULL);
    m_fader = new GenericFader;

    resetElapsed();
}

void EFX::disarm()
{
    Q_ASSERT(m_fader != NULL);
    delete m_fader;
    m_fader = NULL;
}

void EFX::postRun(MasterTimer* timer, UniverseArray* universes)
{
    /* Reset all fixtures */
    QListIterator <EFXFixture*> it(m_fixtures);
    while (it.hasNext() == true)
    {
        EFXFixture* ef(it.next());

        /* Run the EFX's stop scene for Loop & PingPong modes */
        if (m_runOrder != SingleShot)
            ef->stop(timer, universes);
        ef->reset();
    }

    m_fader->removeAll();

    Function::postRun(timer, universes);
}

void EFX::write(MasterTimer* timer, UniverseArray* universes)
{
    int ready = 0;

    Q_UNUSED(timer);

    QListIterator <EFXFixture*> it(m_fixtures);
    while (it.hasNext() == true)
    {
        EFXFixture* ef = it.next();
        if (ef->isReady() == false)
            ef->nextStep(timer, universes);
        else
            ready++;
    }

    incrementElapsed();

    /* Check for stop condition */
    if (ready == m_fixtures.count())
        stop();
    m_fader->write(universes);
}

/*****************************************************************************
 * Intensity
 *****************************************************************************/

void EFX::adjustIntensity(qreal fraction)
{
    if (m_fader != NULL)
        m_fader->adjustIntensity(fraction);

    QListIterator <EFXFixture*> it(m_fixtures);
    while (it.hasNext() == true)
    {
        EFXFixture* ef = it.next();
        ef->adjustIntensity(fraction);
    }

    Function::adjustIntensity(fraction);
}
