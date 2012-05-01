/*
  Q Light Controller
  vcproperties.cpp

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

#include <QWidget>
#include <QtXml>

#include "qlcfile.h"

#include "virtualconsole.h"
#include "vcproperties.h"
#include "outputmap.h"
#include "inputmap.h"
#include "vcframe.h"
#include "doc.h"

/*****************************************************************************
 * Properties Initialization
 *****************************************************************************/

VCProperties::VCProperties()
    : m_gridX(10)
    , m_gridY(10)
    , m_sizeX(1920)
    , m_sizeY(1080)

    , m_tapModifier(Qt::ControlModifier)

    , m_gmChannelMode(UniverseArray::GMIntensity)
    , m_gmValueMode(UniverseArray::GMReduce)
    , m_gmInputUniverse(InputMap::invalidUniverse())
    , m_gmInputChannel(InputMap::invalidChannel())
    , m_gmVisible(true)

    , m_blackoutInputUniverse(InputMap::invalidUniverse())
    , m_blackoutInputChannel(InputMap::invalidChannel())
{
}

VCProperties::VCProperties(const VCProperties& properties)
    : m_gridX(properties.m_gridX)
    , m_gridY(properties.m_gridY)
    , m_sizeX(properties.m_sizeX)
    , m_sizeY(properties.m_sizeY)

    , m_tapModifier(properties.m_tapModifier)

    , m_gmChannelMode(properties.m_gmChannelMode)
    , m_gmValueMode(properties.m_gmValueMode)
    , m_gmInputUniverse(properties.m_gmInputUniverse)
    , m_gmInputChannel(properties.m_gmInputChannel)
    , m_gmVisible(properties.m_gmVisible)

    , m_blackoutInputUniverse(properties.m_blackoutInputUniverse)
    , m_blackoutInputChannel(properties.m_blackoutInputChannel)
{
}

VCProperties::~VCProperties()
{
}

/*****************************************************************************
 * Grid
 *****************************************************************************/

void VCProperties::setGridX(int x)
{
    m_gridX = x;
}

int VCProperties::gridX() const
{
    return m_gridX;
}

void VCProperties::setGridY(int y)
{
    m_gridY = y;
}

int VCProperties::gridY() const
{
    return m_gridY;
}

/*****************************************************************************
 * Size
 *****************************************************************************/

void VCProperties::setSizeX(int x)
{
    m_sizeX = x;
}

int VCProperties::sizeX() const
{
    return m_sizeX;
}

void VCProperties::setSizeY(int y)
{
    m_sizeY = y;
}

int VCProperties::sizeY() const
{
    return m_sizeY;
}

/*****************************************************************************
 * Keyboard
 *****************************************************************************/

void VCProperties::setTapModifier(Qt::KeyboardModifier mod)
{
    m_tapModifier = mod;
}

Qt::KeyboardModifier VCProperties::tapModifier() const
{
    return m_tapModifier;
}

/*****************************************************************************
 * Grand Master
 *****************************************************************************/

void VCProperties::setGMVisible(bool v)
{
    m_gmVisible = v;
}

bool VCProperties::isGMVisible() const
{
    return m_gmVisible;
}

void VCProperties::setGrandMasterChannelMode(UniverseArray::GMChannelMode mode)
{
    m_gmChannelMode = mode;
}

UniverseArray::GMChannelMode VCProperties::grandMasterChannelMode() const
{
    return m_gmChannelMode;
}

void VCProperties::setGrandMasterValueMode(UniverseArray::GMValueMode mode)
{
    m_gmValueMode = mode;
}

UniverseArray::GMValueMode VCProperties::grandMasterValueMode() const
{
    return m_gmValueMode;
}

void VCProperties::setGrandMasterInputSource(quint32 universe, quint32 channel)
{
    m_gmInputUniverse = universe;
    m_gmInputChannel = channel;
}

quint32 VCProperties::grandMasterInputUniverse() const
{
    return m_gmInputUniverse;
}

quint32 VCProperties::grandMasterInputChannel() const
{
    return m_gmInputChannel;
}

/*****************************************************************************
 * Blackout
 *****************************************************************************/

void VCProperties::setBlackoutInputSource(quint32 universe, quint32 channel)
{
    m_blackoutInputUniverse = universe;
    m_blackoutInputChannel = channel;
}

quint32 VCProperties::blackoutInputUniverse() const
{
    return m_blackoutInputUniverse;
}

quint32 VCProperties::blackoutInputChannel() const
{
    return m_blackoutInputChannel;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCProperties::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCVCProperties)
    {
        qWarning() << Q_FUNC_INFO << "Virtual console properties node not found";
        return false;
    }

    QString str;
    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCVCPropertiesGrid)
        {
            /* Grid X resolution */
            str = tag.attribute(KXMLQLCVCPropertiesGridXResolution);
            if (str.isEmpty() == false)
                setGridX(str.toInt());

            /* Grid Y resolution */
            str = tag.attribute(KXMLQLCVCPropertiesGridYResolution);
            if (str.isEmpty() == false)
                setGridY(str.toInt());
        }
        else if (tag.tagName() == KXMLQLCVCPropertiesSize)
        {
            /* Width */
            str = tag.attribute(KXMLQLCVCPropertiesSizeWidth);
            if (str.isEmpty() == false)
                setSizeX(str.toInt());

            /* Height */
            str = tag.attribute(KXMLQLCVCPropertiesSizeHeight);
            if (str.isEmpty() == false)
                setSizeY(str.toInt());
        }
        else if (tag.tagName() == KXMLQLCVCPropertiesKeyboard)
        {
            /* Tap modifier */
            str = tag.attribute(KXMLQLCVCPropertiesKeyboardTapModifier);
            if (str.isEmpty() == false)
                setTapModifier(Qt::KeyboardModifier(str.toInt()));
        }
        else if (tag.tagName() == KXMLQLCVCPropertiesGrandMaster)
        {
            quint32 universe = InputMap::invalidUniverse();
            quint32 channel = InputMap::invalidChannel();

            str = tag.attribute(KXMLQLCVCPropertiesGrandMasterVisible);
            setGMVisible((str == KXMLQLCTrue) ? true : false);

            str = tag.attribute(KXMLQLCVCPropertiesGrandMasterChannelMode);
            setGrandMasterChannelMode(UniverseArray::stringToGMChannelMode(str));

            str = tag.attribute(KXMLQLCVCPropertiesGrandMasterValueMode);
            setGrandMasterValueMode(UniverseArray::stringToGMValueMode(str));

            /* External input */
            if (loadXMLInput(tag.firstChild().toElement(), &universe, &channel) == true)
                setGrandMasterInputSource(universe, channel);
        }
        else if (tag.tagName() == KXMLQLCVCPropertiesBlackout)
        {
            /* External input */
            quint32 universe = InputMap::invalidUniverse();
            quint32 channel = InputMap::invalidChannel();
            if (loadXMLInput(tag.firstChild().toElement(), &universe, &channel) == true)
                setBlackoutInputSource(universe, channel);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown virtual console property tag:" << tag.tagName();
        }

        /* Next node */
        node = node.nextSibling();
    }

    return true;
}

bool VCProperties::saveXML(QDomDocument* doc, QDomElement* wksp_root) const
{
    QDomElement prop_root;
    QDomElement subtag;
    QDomElement tag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    /* Properties entry */
    prop_root = doc->createElement(KXMLQLCVCProperties);
    wksp_root->appendChild(prop_root);

    /* Grid */
    tag = doc->createElement(KXMLQLCVCPropertiesGrid);
    tag.setAttribute(KXMLQLCVCPropertiesGridXResolution, QString("%1").arg(m_gridX));
    tag.setAttribute(KXMLQLCVCPropertiesGridYResolution, QString("%1").arg(m_gridY));
    prop_root.appendChild(tag);

    /* Size */
    tag = doc->createElement(KXMLQLCVCPropertiesSize);
    tag.setAttribute(KXMLQLCVCPropertiesSizeWidth, QString::number(m_gridX));
    tag.setAttribute(KXMLQLCVCPropertiesSizeHeight, QString::number(m_gridY));
    prop_root.appendChild(tag);

    /* Keyboard */
    tag = doc->createElement(KXMLQLCVCPropertiesKeyboard);
    tag.setAttribute(KXMLQLCVCPropertiesKeyboardTapModifier, tapModifier());
    prop_root.appendChild(tag);

    /***********************
     * Grand Master slider *
     ***********************/
    tag = doc->createElement(KXMLQLCVCPropertiesGrandMaster);
    tag.setAttribute(KXMLQLCVCPropertiesGrandMasterVisible, (isGMVisible()) ? KXMLQLCTrue : KXMLQLCFalse);
    prop_root.appendChild(tag);

    /* Channel mode */
    tag.setAttribute(KXMLQLCVCPropertiesGrandMasterChannelMode,
                     UniverseArray::gMChannelModeToString(m_gmChannelMode));

    /* Value mode */
    tag.setAttribute(KXMLQLCVCPropertiesGrandMasterValueMode,
                     UniverseArray::gMValueModeToString(m_gmValueMode));

    /* Grand Master external input */
    if (m_gmInputUniverse != InputMap::invalidUniverse() &&
        m_gmInputChannel != InputMap::invalidChannel())
    {
        subtag = doc->createElement(KXMLQLCVCPropertiesInput);
        tag.appendChild(subtag);
        subtag.setAttribute(KXMLQLCVCPropertiesInputUniverse,
                            QString("%1").arg(m_gmInputUniverse));
        subtag.setAttribute(KXMLQLCVCPropertiesInputChannel,
                            QString("%1").arg(m_gmInputChannel));
    }

    /************
     * Blackout *
     ************/
    tag = doc->createElement(KXMLQLCVCPropertiesBlackout);
    prop_root.appendChild(tag);

    /* Grand Master external input */
    if (m_blackoutInputUniverse != InputMap::invalidUniverse() &&
        m_blackoutInputChannel != InputMap::invalidChannel())
    {
        subtag = doc->createElement(KXMLQLCVCPropertiesInput);
        tag.appendChild(subtag);
        subtag.setAttribute(KXMLQLCVCPropertiesInputUniverse,
                            QString("%1").arg(m_blackoutInputUniverse));
        subtag.setAttribute(KXMLQLCVCPropertiesInputChannel,
                            QString("%1").arg(m_blackoutInputChannel));
    }

    return true;
}

bool VCProperties::loadXMLInput(const QDomElement& tag, quint32* universe, quint32* channel)
{
    /* External input */
    if (tag.tagName() != KXMLQLCVCPropertiesInput)
        return false;

    QString str;

    /* Universe */
    str = tag.attribute(KXMLQLCVCPropertiesInputUniverse);
    if (str.isEmpty() == false)
        *universe = str.toUInt();
    else
        *universe = InputMap::invalidUniverse();

    /* Channel */
    str = tag.attribute(KXMLQLCVCPropertiesInputChannel);
    if (str.isEmpty() == false)
        *channel = str.toUInt();
    else
        *channel = InputMap::invalidChannel();

    /* Verdict */
    if (*universe != InputMap::invalidUniverse() &&
        *channel != InputMap::invalidChannel()) {
        return true;
    } else {
        return false;
    }
}
