/*
  Q Light Controller - Fixture Definition Editor
  avolitesd4parser.h

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

#ifndef AVOLITESD4PARSER_H
#define AVOLITESD4PARSER_H

#include <QDomDocument>
#include <QString>
#include <QList>
#include <QMap>

#include "qlcchannel.h"

class QLCFixtureMode;
class QLCFixtureDef;
class QDomElement;

#define KExtAvolitesFixture ".d4" // Avolites Diamond 4 format

class AvolitesD4Parser
{
public:
    /** Create a new Avolites D4 Parser object */
    AvolitesD4Parser();
    ~AvolitesD4Parser();

    /**
     * Load a D4 file from the given path
     *
     * @param path The file path to load from
     * @return true if successful, otherwise false (see lastError() for a possible cause)
     */
    bool loadXML(const QString& path);

    /**
     * After loading a file with loadXML(), convert the loaded D4 file
     * into a QLC Fixture Definition, given in $fixtureDef.
     *
     * @param fixtureDef The fixture definition object to fill, must not be NULL.
     * @return true if successful, otherwise false (see lastError() for possible cause)
     */
    bool fillFixtureDef(QLCFixtureDef *fixtureDef);

    /**
     * Get the last error encountered while loading/parsing a file.
     *
     * @return An error description
     */
    QString lastError() const;

private:
    QString fixtureName() const;
    QString fixtureShortName() const;
    QString fixtureCompany() const;
    QString copyright() const;

    QLCChannel::Group getGroupFromXML(const QDomElement& elem);
    QLCChannel::PrimaryColour getColourFromXML(const QDomElement& elem);

private:
    enum Attributes
    {
        SPECIAL,
        INTENSITY,
        PANTILT,
        COLOUR,
        GOBO,
        BEAM,
        EFFECT
    };

    typedef QMap <QString,Attributes> StringToEnumMap;
    typedef QMap <QString,QLCChannel*> ChannelsMap;

private:
    bool isFunction(const QDomElement& elem) const;
    bool is16Bit(const QDomElement& elem) const;
    bool parseChannels(const QDomElement& elem, QLCFixtureDef *fixtureDef);
    bool parseCapabilities(const QDomElement& elem, QLCChannel *chan, bool isFine = false);
    bool parseModes(const QDomElement& elem, QLCFixtureDef *fixtureDef);
    void parsePhysical(const QDomElement& elem, QLCFixtureMode *mode);

    Attributes stringToAttributeEnum(const QString& attr);

    QString guessType(const QLCFixtureDef* def) const;

private:
    QString m_lastError;
    QDomDocument m_documentRoot;
    ChannelsMap m_channels;

    static StringToEnumMap m_attributesMap;
};

#endif // AVOLITESD4PARSER_H
