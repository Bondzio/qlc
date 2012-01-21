/*
  Q Light Controller - Fixture Definition Editor
  avolitesd4parser.cpp

  Copyright (C) Rui Barreiros
                Heikki Junnila

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
#include <QStringList>
#include <QDebug>

#include "avolitesd4parser.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlccapability.h"
#include "qlcphysical.h"
#include "qlcchannel.h"
#include "qlcfile.h"

AvolitesD4Parser::StringToEnumMap AvolitesD4Parser::m_attributesMap;

AvolitesD4Parser::AvolitesD4Parser()
{
    if (m_attributesMap.isEmpty() == true)
    {
        // Setup our attribute mapping map helper
        m_attributesMap.insert("S", AvolitesD4Parser::SPECIAL);
        m_attributesMap.insert("I", AvolitesD4Parser::INTENSITY);
        m_attributesMap.insert("P", AvolitesD4Parser::PANTILT);
        m_attributesMap.insert("C", AvolitesD4Parser::COLOUR);
        m_attributesMap.insert("G", AvolitesD4Parser::GOBO);
        m_attributesMap.insert("B", AvolitesD4Parser::BEAM);
        m_attributesMap.insert("E", AvolitesD4Parser::EFFECT);
    }
}

AvolitesD4Parser::~AvolitesD4Parser()
{
}

bool AvolitesD4Parser::loadXML(const QString& path)
{
    m_lastError = QString();
    m_documentRoot = QDomDocument();
    m_channels.clear();

    if (path.isEmpty())
    {
        m_lastError = "filename not specified";
        return false;
    }

    m_documentRoot = QLCFile::readXML(path);
    if (m_documentRoot.isNull() == true)
    {
        m_lastError = "unable to read document";
        return false;
    }

    // check if the document has <Fixture></Fixture> if not then it's not a valid file
    QDomElement el = m_documentRoot.namedItem("Fixture").toElement();
    if (el.isNull() && (!el.hasAttribute("Name") ||
        !el.hasAttribute("Shortname") || !el.hasAttribute("Company")))
    {
        m_lastError = "wrong document format";
        return false;
    }

    return true;
}

bool AvolitesD4Parser::fillFixtureDef(QLCFixtureDef* fixtureDef)
{
    if (m_documentRoot.isNull())
    {
        m_lastError = "no XML loaded to process";
        return false;
    }

    fixtureDef->setManufacturer(fixtureCompany());
    fixtureDef->setModel(fixtureName());
    fixtureDef->setAuthor(copyright());

    // Parse all channels
    if (!parseChannels(m_documentRoot.namedItem("Fixture").toElement().namedItem("Control").toElement(), fixtureDef))
        return false;

    // Parse all modes
    if (!parseModes(m_documentRoot.namedItem("Fixture").toElement(), fixtureDef))
        return false;

    fixtureDef->setType(guessType(fixtureDef));

    // TODO TODO TODO
    // Maybe also import preset palettes and macros ?!?!?!?!

    return true;
}

QString AvolitesD4Parser::lastError() const
{
    return m_lastError;
}

QString AvolitesD4Parser::fixtureName() const
{
    if (m_documentRoot.isNull())
        return QString();
    else
        return m_documentRoot.namedItem("Fixture").toElement().attribute("Name");
}

QString AvolitesD4Parser::fixtureShortName() const
{
    if (m_documentRoot.isNull())
        return QString();
    else
        return m_documentRoot.namedItem("Fixture").toElement().attribute("ShortName");
}

QString AvolitesD4Parser::fixtureCompany() const
{
    if (m_documentRoot.isNull())
        return QString();
    else
        return m_documentRoot.namedItem("Fixture").toElement().attribute("Company");
}

QString AvolitesD4Parser::copyright() const
{
    return QString("Avolites");
}

QLCChannel::Group AvolitesD4Parser::getGroupFromXML(const QDomElement& elem)
{
    if (elem.isNull())
        return QLCChannel::NoGroup;

    switch (stringToAttributeEnum(elem.attribute("Group")))
    {
    case AvolitesD4Parser::SPECIAL:
        if (elem.attribute("ID").contains("Speed", Qt::CaseInsensitive)
                || elem.attribute("Name").contains("Speed", Qt::CaseInsensitive))
            return QLCChannel::Speed;
        else if (elem.attribute("ID").contains("Macro", Qt::CaseInsensitive)
                || elem.attribute("Name").contains("Macro", Qt::CaseInsensitive))
            return QLCChannel::Effect;
        else if (elem.attribute("ID").contains("Reserved", Qt::CaseInsensitive)
                || elem.attribute("Name").contains("Reserved", Qt::CaseInsensitive))
            return QLCChannel::NoGroup;
        else
            return QLCChannel::Maintenance;
        break;

    default:
    case AvolitesD4Parser::INTENSITY:
        if (elem.attribute("ID").contains("Shutter", Qt::CaseInsensitive)
                || elem.attribute("Name").contains("Shutter", Qt::CaseInsensitive))
            return QLCChannel::Shutter;
        else
            return QLCChannel::Intensity;
        break;

    case AvolitesD4Parser::PANTILT:
        if (elem.attribute("ID").contains("Pan", Qt::CaseInsensitive)
                || elem.attribute("Name").contains("Pan", Qt::CaseInsensitive))
            return QLCChannel::Pan;
        else if (elem.attribute("ID").contains("Tilt", Qt::CaseInsensitive)
                || elem.attribute("Name").contains("Tilt", Qt::CaseInsensitive))
            return QLCChannel::Tilt;
        else
            return QLCChannel::NoGroup;
        break;

    case AvolitesD4Parser::COLOUR:
        if (elem.attribute("ID").contains("Cyan", Qt::CaseInsensitive)
                || elem.attribute("Name").contains("Cyan", Qt::CaseInsensitive))
            return QLCChannel::Intensity;
        else if (elem.attribute("ID").contains("Magenta", Qt::CaseInsensitive)
                || elem.attribute("Name").contains("Magenta", Qt::CaseInsensitive))
            return QLCChannel::Intensity;
        else if (elem.attribute("ID").contains("Yellow", Qt::CaseInsensitive)
                || elem.attribute("Name").contains("Yellow", Qt::CaseInsensitive))
            return QLCChannel::Intensity;
        else if (elem.attribute("ID").contains("Red", Qt::CaseInsensitive)
                || elem.attribute("Name").contains("Red", Qt::CaseInsensitive))
            return QLCChannel::Intensity;
        else if (elem.attribute("ID").contains("Green", Qt::CaseInsensitive)
                || elem.attribute("Name").contains("Gree", Qt::CaseInsensitive))
            return QLCChannel::Intensity;
        else if (elem.attribute("ID").contains("Blue", Qt::CaseInsensitive)
                || elem.attribute("Name").contains("Blue", Qt::CaseInsensitive))
            return QLCChannel::Intensity;
        else
            return QLCChannel::Colour;
        break;

    case AvolitesD4Parser::GOBO:
        return QLCChannel::Gobo;
        break;

    case AvolitesD4Parser::BEAM:
        return QLCChannel::Beam;
        break;

    case AvolitesD4Parser::EFFECT:
        if (elem.attribute("ID").contains("Prism", Qt::CaseInsensitive)
                || elem.attribute("Name").contains("Prism", Qt::CaseInsensitive))
            return QLCChannel::Prism;
        else if (elem.attribute("ID").contains("Effect", Qt::CaseInsensitive)
                || elem.attribute("Name").contains("Effect", Qt::CaseInsensitive))
            return QLCChannel::Effect;
        else
            return QLCChannel::NoGroup;
        break;
    }

    return QLCChannel::NoGroup;
}

QLCChannel::PrimaryColour AvolitesD4Parser::getColourFromXML(const QDomElement& elem)
{
    if (elem.attribute("Group").compare("C", Qt::CaseInsensitive) != 0)
        return QLCChannel::NoColour;

    if (elem.attribute("ID").contains("Cyan", Qt::CaseInsensitive)
            || elem.attribute("Name").contains("Cyan", Qt::CaseInsensitive))
        return QLCChannel::Cyan;
    else if (elem.attribute("ID").contains("Magenta", Qt::CaseInsensitive)
            || elem.attribute("Name").contains("Magenta", Qt::CaseInsensitive))
        return QLCChannel::Magenta;
    else if (elem.attribute("ID").contains("Yellow", Qt::CaseInsensitive)
            || elem.attribute("Name").contains("Yellow", Qt::CaseInsensitive))
        return QLCChannel::Yellow;
    else if (elem.attribute("ID").contains("Red", Qt::CaseInsensitive)
            || elem.attribute("Name").contains("Red", Qt::CaseInsensitive))
        return QLCChannel::Red;
    else if (elem.attribute("ID").contains("Green", Qt::CaseInsensitive)
            || elem.attribute("Name").contains("Green", Qt::CaseInsensitive))
        return QLCChannel::Green;
    else if (elem.attribute("ID").contains("Blue", Qt::CaseInsensitive)
            || elem.attribute("Name").contains("Blue", Qt::CaseInsensitive))
        return QLCChannel::Blue;
    else
        return QLCChannel::NoColour;
}

bool AvolitesD4Parser::isFunction(const QDomElement& elem) const
{
    QDomElement el = elem.firstChildElement("Function");
    for (; !el.isNull(); el = el.nextSiblingElement("Function"))
    {
        if (!el.attribute("Update").isEmpty())
            return true;
    }

    return false;
}

bool AvolitesD4Parser::is16Bit(const QDomElement& elem) const
{
    QDomElement el = elem.firstChildElement("Function");
    for (; !el.isNull(); el = el.nextSiblingElement("Function"))
    {
        QString dmx = el.attribute("Dmx");
        QStringList dmxValues = dmx.split('~');

        if (dmxValues.value(1).toInt() > 256)
            return true;
    }

    return false;
}

bool AvolitesD4Parser::parseChannels(const QDomElement &elem, QLCFixtureDef* fixtureDef)
{
    QDomElement el = elem.firstChildElement("Attribute");
    for (; !el.isNull(); el = el.nextSiblingElement("Attribute"))
    {
        // Small integrity check
        if (el.attribute("ID").isEmpty())
            continue;

        // If this attribute is a function (i.e. an attribute used as a control variable for other attributes)
        // then we just ignore it and continue. We can check it by checking if attribute Update on a <Function/> exists
        if (isFunction(el))
            continue;

        QLCChannel* chan = new QLCChannel();
        chan->setName(el.attribute("Name"));
        chan->setGroup(getGroupFromXML(el));
        chan->setColour(getColourFromXML(el));
        chan->setControlByte(QLCChannel::MSB);

        // add channel to fixture definition
        fixtureDef->addChannel(chan);
        m_channels.insert(el.attribute("ID"), chan);

        // if this channel is a NoGroup then we don't need to continue
        // no capabilities nor 16 bit channel
        if (chan->group() == QLCChannel::NoGroup)
            continue;

        // parse capabilities
        if (!parseCapabilities(el, chan))
        {
            m_channels.remove(el.attribute("ID"));
            delete chan;
            return false;
        }

        // If we have a DMX attribute higher than 255 means we have an attribute with a 16bit precision
        // so, we add another channel, with 'Fine' appended to it's name and set the LSB controlbyte
        if (is16Bit(el))
        {
            QLCChannel* fchan = new QLCChannel();
            fchan->setName(el.attribute("Name") + " Fine");
            fchan->setGroup(getGroupFromXML(el));
            fchan->setColour(getColourFromXML(el));
            fchan->setControlByte(QLCChannel::LSB);

            // parse capabilities
            if (!parseCapabilities(el, fchan, true))
            {
                delete fchan;
                return false;
            }

            // Finally add channel to fixture definition
            fixtureDef->addChannel(fchan);
            m_channels.insert(el.attribute("ID") + " Fine", fchan);
        }
    }

    return true;
}

bool AvolitesD4Parser::parseCapabilities(const QDomElement &elem, QLCChannel* chan, bool isFine)
{
    QDomElement el = elem.firstChildElement("Function");
    for (; !el.isNull(); el = el.nextSiblingElement("Function"))
    {
        // Small integrity check
        if (el.attribute("Name").isEmpty())
            continue;

        QString dmx = el.attribute("Dmx");
        QStringList dmxValues = dmx.split('~');

        // if were trying to get capabilities from a 16bit channel, we need to change them to 8 bit
        int minValue = 0, maxValue = 0;

        if (dmxValues.value(0).toInt() > 256)
            minValue = 0xFF & (dmxValues.value(0).toInt() >> 8);
        else
            minValue = dmxValues.value(0).toInt();

        if (dmxValues.value(1).toInt() > 256)
            maxValue = 0xFF & (dmxValues.value(1).toInt() >> 8);
        else
            maxValue = dmxValues.value(1).toInt();

        // Guess what, I seen this happen, it seems min value is not always on the left of the ~
        // sometimes they're switched!!!!!!!!!!!
        if (minValue > maxValue)
        {
            int tmp = maxValue;
            maxValue = minValue;
            minValue = tmp;
        }

        QString name = el.attribute("Name");
        if (isFine)
            name += " Fine";

        QLCCapability* cap = new QLCCapability(minValue, maxValue, name);

        // We just ignore capability adding errors, because avolites often repeats attributes due to conditionals
        // so we just add the first one we get, the repeating ones are ignored naturally and
        // obviously further human verification is needed on the fixture definition to fix this issues
        chan->addCapability(cap);
    }

    return true;
}

bool AvolitesD4Parser::parseModes(const QDomElement &elem, QLCFixtureDef* fixtureDef)
{
    QDomElement el = elem.firstChildElement("Mode");
    for (; !el.isNull(); el = el.nextSiblingElement("Mode"))
    {
        if (el.attribute("Name").isEmpty())
            continue;

        QLCFixtureMode* mode = new QLCFixtureMode(fixtureDef);
        mode->setName(el.attribute("Name"));

        // Parse physical
        parsePhysical(el.namedItem("Physical").toElement(), mode);

        QMap <int,QLCChannel*> channelList;
        QDomElement e = el.namedItem("Include").toElement().firstChildElement("Attribute");
        for (; !e.isNull(); e = e.nextSiblingElement("Attribute"))
        {
            // Some channels are conditionals not real channels
            if (e.attribute("ChannelOffset").isEmpty())
                continue;

            if (m_channels.contains(e.attribute("ID")))
            {
                // might be a 16 bit channel, so we have 2 DMX addresses
                QString dmx = e.attribute("ChannelOffset");
                if (dmx.contains(",", Qt::CaseInsensitive))
                {
                    // 16 bit address, we need to add 2 channels, this one, and we need the fine one
                    QStringList dmxValues = dmx.split(",");
                    // if there's more than 2 addresses, bail out, don't know how to handle this, shouldn't be
                    if (dmxValues.count() > 2)
                        continue;

                    // Add this one
                    channelList.insert(dmxValues.value(0).toInt(), m_channels.value(e.attribute("ID")));
                    QString name = m_channels.value(e.attribute("ID"))->name();

                    // Search for the fine one
                    QMapIterator <QString,QLCChannel*> it(m_channels);
                    while (it.hasNext() == true)
                    {
                        it.next();
                        QLCChannel* ch(it.value());
                        Q_ASSERT(ch != NULL);

                        if (ch->name() == QString(name + " Fine"))
                            channelList.insert(dmxValues.value(1).toInt(), ch);
                    }
                }
                else
                {
                    channelList.insert(dmx.toInt(), m_channels.value(e.attribute("ID")));
                }
            }
        }

        QMapIterator <int,QLCChannel*> it(channelList);
        while (it.hasNext() == true)
        {
            it.next();
            Q_ASSERT(mode != NULL);
            mode->insertChannel(it.value(), it.key());
        }

        // Add the mode
        fixtureDef->addMode(mode);
    }

    return true;
}

void AvolitesD4Parser::parsePhysical(const QDomElement &el, QLCFixtureMode* mode)
{
    QLCPhysical phys;
    phys.setBulbType(el.namedItem("Bulb").toElement().attribute("Type"));
    phys.setBulbLumens(el.namedItem("Bulb").toElement().attribute("Lumens").toInt());
    phys.setBulbColourTemperature(el.namedItem("Bulb").toElement().attribute("ColourTemp").toInt());
    phys.setLensName(el.namedItem("Lens").toElement().attribute("Name"));

    QString degrees = el.namedItem("Lens").toElement().attribute("Degrees");
    if (degrees.contains("~"))
    {
        QStringList deg = degrees.split('~');
        if (deg.value(0).toInt() > deg.value(1).toInt())
        {
            phys.setLensDegreesMin(deg.value(1).toInt());
            phys.setLensDegreesMax(deg.value(0).toInt());
        }
        else
        {
            phys.setLensDegreesMin(deg.value(0).toInt());
            phys.setLensDegreesMax(deg.value(1).toInt());
        }
    }
    else if (!degrees.isEmpty())
    {
        phys.setLensDegreesMax(degrees.toInt());
        phys.setLensDegreesMin(degrees.toInt());
    }

    phys.setWeight(el.namedItem("Weight").toElement().attribute("Kg").toDouble());

    phys.setHeight((int)(el.namedItem("Size").toElement().attribute("Height").toDouble() * 1000));
    phys.setWidth((int)(el.namedItem("Size").toElement().attribute("Width").toDouble() * 1000));
    phys.setDepth((int)(el.namedItem("Size").toElement().attribute("Depth").toDouble() * 1000));

    phys.setFocusType(el.namedItem("Focus").toElement().attribute("Type"));
    phys.setFocusPanMax(el.namedItem("Focus").toElement().attribute("PanMax").toInt());
    phys.setFocusTiltMax(el.namedItem("Focus").toElement().attribute("TiltMax").toInt());

    mode->setPhysical(phys);
}

AvolitesD4Parser::Attributes AvolitesD4Parser::stringToAttributeEnum(const QString& attr)
{
    // If there is none, empty or whatever always return something, default is SPECIAL
    if (attr.isEmpty())
        return AvolitesD4Parser::SPECIAL;

    if (m_attributesMap.value(attr.toUpper()))
        return m_attributesMap.value(attr.toUpper());
    else
        return AvolitesD4Parser::SPECIAL;
}

QString AvolitesD4Parser::guessType(const QLCFixtureDef* def) const
{
    Q_ASSERT(def != NULL);

    int pan = 0, tilt = 0;
    int r = 0, g = 0, b = 0, c = 0, m = 0, y = 0, nocol = 0;
    int gobo = 0, colour = 0;
    int haze = 0, smoke = 0;
    int strobe = 0;

    QListIterator <QLCChannel*> it(def->channels());
    while (it.hasNext() == true)
    {
        const QLCChannel* ch(it.next());
        if (ch->group() == QLCChannel::Pan)
        {
            pan++;
        }
        else if (ch->group() == QLCChannel::Tilt)
        {
            tilt++;
        }
        else if (ch->group() == QLCChannel::Intensity)
        {
            if (ch->colour() == QLCChannel::Red)
                r++;
            else if (ch->colour() == QLCChannel::Green)
                g++;
            else if (ch->colour() == QLCChannel::Blue)
                b++;
            else if (ch->colour() == QLCChannel::Cyan)
                c++;
            else if (ch->colour() == QLCChannel::Magenta)
                m++;
            else if (ch->colour() == QLCChannel::Yellow)
                y++;
            else
                nocol++;
        }
        else if (ch->group() == QLCChannel::Shutter)
        {
            if (ch->searchCapability(/*S/s*/"trobe", false) != NULL)
                strobe++;
        }
        else if (ch->group() == QLCChannel::Gobo)
        {
            gobo++;
        }
        else if (ch->group() == QLCChannel::Colour)
        {
            colour++;
        }
        else if (ch->name().contains("strobe", Qt::CaseInsensitive) == true)
        {
            strobe++;
        }
        else if (ch->name().contains("haze", Qt::CaseInsensitive) == true)
        {
            haze++;
        }
        else if (ch->name().contains("smoke", Qt::CaseInsensitive) == true)
        {
            smoke++;
        }
    }

    if (pan >= 2 && tilt >= 2)
        return QString("Moving Head"); // Quite probable, few scanners with 16bit addressing
    else if (pan == 1 && tilt == 1)
        return QString("Scanner"); // Quite probable, though some moving heads are only 8bit
    else if (gobo > 0)
        return QString("Flower"); // No pan/tilt, but gobo, fairly certain
    else if (colour > 0 || (r > 0 && g > 0 && b > 0) || (c > 0 && m > 0 && y > 0))
        return QString("Color Changer"); // No pan/tilt/gobos, but RGB/CMY mixing or dichro
    else if (strobe > 0)
        return QString("Strobe"); // Duh.
    else if (smoke > 0)
        return QString("Smoke"); // Duh.
    else if (nocol > 0)
        return QString("Dimmer"); // Kinda..mmmmh..
    else
        return QString("Other"); // Give up
}
