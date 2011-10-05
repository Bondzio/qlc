/*
  Q Light Controller
  efxfixture.cpp

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
#include <math.h>

#include "universearray.h"
#include "genericfader.h"
#include "mastertimer.h"
#include "efxfixture.h"
#include "function.h"
#include "scene.h"
#include "efx.h"
#include "doc.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

EFXFixture::EFXFixture(EFX* parent)
{
    Q_ASSERT(parent != NULL);
    m_parent = parent;

    m_fixture = Fixture::invalidId();
    m_serialNumber = 0;
    m_direction = Function::Forward;
    m_runTimeDirection = Function::Forward;
    m_ready = false;
    m_started = false;

    m_skipIterator = 0;
    m_skipThreshold = 0;
    m_iterator = 0;
    m_panValue = 0;
    m_tiltValue = 0;

    m_intensity = 1.0;

    m_fadeIntensity = 255;
}

void EFXFixture::copyFrom(const EFXFixture* ef)
{
    /* Don't copy m_parent because it is assigned in constructor. */

    m_fixture = ef->m_fixture;
    m_serialNumber = ef->m_serialNumber;
    m_direction = ef->m_direction;
    m_runTimeDirection = ef->m_runTimeDirection;
    m_ready = ef->m_ready;
    m_started = ef->m_started;

    m_skipIterator = ef->m_skipIterator;
    m_skipThreshold = ef->m_skipThreshold;
    m_iterator = ef->m_iterator;
    m_panValue = ef->m_panValue;
    m_tiltValue = ef->m_tiltValue;

    m_intensity = ef->m_intensity;

    m_fadeIntensity = ef->m_fadeIntensity;
}

EFXFixture::~EFXFixture()
{
}

Doc* EFXFixture::doc() const
{
    Q_ASSERT(m_parent != NULL);
    return m_parent->doc();
}

/****************************************************************************
 * Public properties
 ****************************************************************************/

void EFXFixture::setFixture(quint32 fxi_id)
{
    m_fixture = fxi_id;
}

quint32 EFXFixture::fixture() const
{
    return m_fixture;
}

void EFXFixture::setDirection(Function::Direction dir)
{
    m_direction = dir;
    m_runTimeDirection = dir;
}

Function::Direction EFXFixture::direction() const
{
    return m_direction;
}

void EFXFixture::setFadeIntensity(uchar value)
{
    m_fadeIntensity = value;
}

uchar EFXFixture::fadeIntensity() const
{
    return m_fadeIntensity;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool EFXFixture::loadXML(const QDomElement* root)
{
    QDomElement tag;
    QDomNode node;

    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCEFXFixture)
    {
        qWarning("EFX Fixture node not found!");
        return false;
    }

    /* New file format contains sub tags */
    node = root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();

        if (tag.tagName() == KXMLQLCEFXFixtureID)
        {
            /* Fixture ID */
            setFixture(tag.text().toInt());
        }
        else if (tag.tagName() == KXMLQLCEFXFixtureDirection)
        {
            /* Direction */
            Function::Direction dir;
            dir = Function::stringToDirection(tag.text());
            setDirection(dir);
        }
        else if (tag.tagName() == KXMLQLCEFXFixtureIntensity)
        {
            /* Intensity */
            setFadeIntensity(uchar(tag.text().toUInt()));
        }
        else
        {
            qWarning() << "Unknown EFX Fixture tag:" << tag.tagName();
        }
        node = node.nextSibling();
    }

    return true;
}

bool EFXFixture::saveXML(QDomDocument* doc, QDomElement* efx_root) const
{
    QDomElement subtag;
    QDomElement tag;
    QDomText text;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(efx_root != NULL);

    /* EFXFixture */
    tag = doc->createElement(KXMLQLCEFXFixture);
    efx_root->appendChild(tag);

    /* Fixture ID */
    subtag = doc->createElement(KXMLQLCEFXFixtureID);
    tag.appendChild(subtag);
    text = doc->createTextNode(QString("%1").arg(fixture()));
    subtag.appendChild(text);

    /* Direction */
    subtag = doc->createElement(KXMLQLCEFXFixtureDirection);
    tag.appendChild(subtag);
    text = doc->createTextNode(Function::directionToString(m_direction));
    subtag.appendChild(text);

    /* Intensity */
    subtag = doc->createElement(KXMLQLCEFXFixtureIntensity);
    tag.appendChild(subtag);
    text = doc->createTextNode(QString::number(fadeIntensity()));
    subtag.appendChild(text);

    return true;
}

/****************************************************************************
 * Protected run-time-only properties
 ****************************************************************************/

void EFXFixture::setSerialNumber(int number)
{
    m_serialNumber = number;
}

int EFXFixture::serialNumber() const
{
    return m_serialNumber;
}

void EFXFixture::updateSkipThreshold()
{
    Q_ASSERT(m_parent != NULL);
    Q_ASSERT(m_parent->fixtures().size() > 0);

    /* One EFX "round" is always (pi * 2) long. Divide this "circumference"
       into as many steps as there are fixtures in this EFX. If there are
       four fixtures, these steps end up in 12 o'clock, 3 o'clock,
       6 o'clock and 9 o'clock etc.. This skip threshold will specify the
       point in the circumference, where the current EFXFixture will come
       along to the EFX. */
    m_skipThreshold = qreal(m_serialNumber) *
                      (qreal(M_PI * 2.0) / qreal(m_parent->fixtures().size()));
}

bool EFXFixture::isValid()
{
    Fixture* fxi = doc()->fixture(fixture());
    if (fxi == NULL)
        return false;
    else if (fxi->panMsbChannel() == QLCChannel::invalid() && // Maybe a device can pan OR tilt
             fxi->tiltMsbChannel() == QLCChannel::invalid())   // but not both. Teh sux0r.
        return false;
    else
        return true;
}

void EFXFixture::reset()
{
    m_panValue = 0;
    m_tiltValue = 0;
    m_skipIterator = 0;
    m_iterator = 0;
    m_ready = false;
    m_runTimeDirection = m_direction;
    m_started = false;

    updateSkipThreshold();
}

bool EFXFixture::isReady() const
{
    return m_ready;
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void EFXFixture::nextStep(MasterTimer* timer, UniverseArray* universes)
{
    /* Bail out without doing anything if this EFX is ready
      (after single-shot), or it has no pan&tilt channels (not valid). */
    if (m_ready == true || isValid() == false)
        return;

    if (m_iterator == 0 && m_started == false)
    {
        updateSkipThreshold();
        if (m_parent->propagationMode() == EFX::Asymmetric)
            m_iterator = m_skipThreshold;
    }

    if (m_parent->propagationMode() == EFX::Serial &&
        m_skipIterator < m_skipThreshold)
    {
        /* Fixture still needs to wait for its turn in serial mode */
        quint32 busValue = Bus::instance()->value(m_parent->bus());
        m_skipIterator += qreal(1) / (qreal(busValue) / qreal(M_PI * 2));
    }
    else
    {
        start(timer, universes);
    }

    if (m_iterator < (M_PI * 2.0))
    {
        if (m_parent->propagationMode() == EFX::Serial)
        {
            if (m_skipIterator >= m_skipThreshold)
            {
                /* Increment for next round. */
                quint32 busValue = Bus::instance()->value(m_parent->bus());
                m_iterator += qreal(1) / (qreal(busValue) / qreal(M_PI * 2));
            }
        }
        else
        {
            /* Increment for next round. */
            quint32 busValue = Bus::instance()->value(m_parent->bus());
            m_iterator += qreal(1) / (qreal(busValue) / qreal(M_PI * 2));
        }

        if (m_runTimeDirection == Function::Forward)
        {
            m_parent->calculatePoint(m_iterator, &m_panValue, &m_tiltValue);
        }
        else
        {
            m_parent->calculatePoint((M_PI * 2.0) - m_iterator, &m_panValue, &m_tiltValue);
        }

        /* Write this fixture's data to universes. */
        setPoint(universes);
    }
    else
    {
        if (m_parent->runOrder() == Function::PingPong)
        {
            /* Reverse direction for ping-pong EFX. */
            if (m_runTimeDirection == Function::Forward)
            {
                m_runTimeDirection = Function::Backward;
            }
            else
            {
                m_runTimeDirection = Function::Forward;
            }
        }
        else if (m_parent->runOrder() == Function::SingleShot)
        {
            /* De-initialize the fixture and mark as ready. */
            m_ready = true;
            stop(timer, universes);
        }

        /* Reset iterator, since we've gone a full cycle. */
        m_iterator = 0;
    }
}

void EFXFixture::start(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(universes);
    Q_UNUSED(timer);

    if (fadeIntensity() > 0 && m_started == false)
    {
        Fixture* fxi = doc()->fixture(fixture());
        Q_ASSERT(fxi != NULL);

        if (fxi->masterIntensityChannel() != QLCChannel::invalid())
        {
            FadeChannel fc;
            fc.setFixture(fixture());
            fc.setChannel(fxi->masterIntensityChannel());
            fc.setBus(m_parent->fadeBusID());

            fc.setStart(0);
            fc.setCurrent(fc.start());
            // Don't use intensity() multiplier because EFX's GenericFader takes care of that
            fc.setTarget(fadeIntensity());
            // Fade channel up with EFX's own GenericFader to allow manual intensity control
            m_parent->m_fader->add(fc);
        }
    }

    m_started = true;
}

void EFXFixture::stop(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(universes);

    if (fadeIntensity() > 0 && m_started == true)
    {
        Fixture* fxi = doc()->fixture(fixture());
        Q_ASSERT(fxi != NULL);

        if (fxi->masterIntensityChannel() != QLCChannel::invalid())
        {
            FadeChannel fc;
            fc.setFixture(fixture());
            fc.setChannel(fxi->masterIntensityChannel());
            fc.setBus(m_parent->fadeBusID());

            fc.setStart(uchar(floor((qreal(fadeIntensity()) * intensity()) + 0.5)));
            fc.setCurrent(fc.start());
            fc.setTarget(0);
            // Give zero-fading to MasterTimer because EFX will stop after this call
            timer->fader()->add(fc);
            // Remove the previously up-faded channel from EFX's internal fader to allow
            // MasterTimer's fader take HTP precedence.
            m_parent->m_fader->remove(fc);
        }
    }

    m_started = false;
}

void EFXFixture::setPoint(UniverseArray* universes)
{
    Q_ASSERT(universes != NULL);

    Fixture* fxi = doc()->fixture(fixture());
    Q_ASSERT(fxi != NULL);

    /* Write coarse point data to universes */
    if (fxi->panMsbChannel() != QLCChannel::invalid())
        universes->write(fxi->universeAddress() + fxi->panMsbChannel(),
                         static_cast<char>(m_panValue), QLCChannel::Pan);
    if (fxi->tiltMsbChannel() != QLCChannel::invalid())
        universes->write(fxi->universeAddress() + fxi->tiltMsbChannel(),
                         static_cast<char> (m_tiltValue), QLCChannel::Tilt);

    /* Write fine point data to universes if applicable */
    if (fxi->panLsbChannel() != QLCChannel::invalid())
    {
        /* Leave only the fraction */
        char value = static_cast<char> ((m_panValue - floor(m_panValue))
                                        * double(UCHAR_MAX));
        universes->write(fxi->universeAddress() + fxi->panLsbChannel(),
                         value, QLCChannel::Pan);
    }

    if (fxi->tiltLsbChannel() != QLCChannel::invalid())
    {
        /* Leave only the fraction */
        char value = static_cast<char> ((m_tiltValue - floor(m_tiltValue))
                                        * double(UCHAR_MAX));
        universes->write(fxi->universeAddress() + fxi->tiltLsbChannel(),
                         value, QLCChannel::Tilt);
    }
}

/*****************************************************************************
 * Intensity
 *****************************************************************************/

void EFXFixture::adjustIntensity(qreal fraction)
{
    m_intensity = fraction;
}

qreal EFXFixture::intensity() const
{
    return m_intensity;
}
