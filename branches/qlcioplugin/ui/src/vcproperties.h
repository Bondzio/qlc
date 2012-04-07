/*
  Q Light Controller
  vcproperties.h

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

#ifndef VCPROPERTIES_H
#define VCPROPERTIES_H

#include "universearray.h"

class VirtualConsole;
class QDomDocument;
class QDomElement;
class MasterTimer;
class OutputMap;
class InputMap;
class VCFrame;
class QWidget;
class Doc;

#define KXMLQLCVirtualConsole "VirtualConsole"

#define KXMLQLCVCProperties "Properties"
#define KXMLQLCVCPropertiesGrid "Grid"
#define KXMLQLCVCPropertiesGridEnabled "Enabled"
#define KXMLQLCVCPropertiesGridXResolution "XResolution"
#define KXMLQLCVCPropertiesGridYResolution "YResolution"

#define KXMLQLCVCPropertiesKeyboard "Keyboard"
#define KXMLQLCVCPropertiesKeyboardGrab "Grab"
#define KXMLQLCVCPropertiesKeyboardRepeatOff "RepeatOff"
#define KXMLQLCVCPropertiesKeyboardTapModifier "TapModifier"

#define KXMLQLCVCPropertiesGrandMaster "GrandMaster"
#define KXMLQLCVCPropertiesGrandMasterVisible "Visible"
#define KXMLQLCVCPropertiesGrandMasterChannelMode "ChannelMode"
#define KXMLQLCVCPropertiesGrandMasterValueMode "ValueMode"

#define KXMLQLCVCPropertiesBlackout "Blackout"

#define KXMLQLCVCPropertiesInput "Input"
#define KXMLQLCVCPropertiesInputUniverse "Universe"
#define KXMLQLCVCPropertiesInputChannel "Channel"

/*****************************************************************************
 * Properties
 *****************************************************************************/

class VCProperties
{
public:
    VCProperties();
    VCProperties(const VCProperties& properties);
    ~VCProperties();

    /*********************************************************************
     * Grid
     *********************************************************************/
public:
    void setGridEnabled(bool enable);
    bool isGridEnabled() const;

    void setGridX(int x);
    int gridX() const;

    void setGridY(int y);
    int gridY() const;

private:
    bool m_gridEnabled;
    int m_gridX;
    int m_gridY;

    /*********************************************************************
     * Keyboard
     *********************************************************************/
public:
    /** Set key repeat off during operate mode or not. */
    void setKeyRepeatOff(bool set);

    /** Check, if key repeat is off during operate mode. */
    bool isKeyRepeatOff() const;

    /** Grab keyboard in operate mode or not. */
    void setGrabKeyboard(bool grab);

    /** Check, if keyboard is grabbed in operate mode. */
    bool isGrabKeyboard() const;

    /** Set the tap modifier key */
    void setTapModifier(Qt::KeyboardModifier mod);

    /** Get the tap modifier key */
    Qt::KeyboardModifier tapModifier() const;

private:
    bool m_keyRepeatOff;
    bool m_grabKeyboard;
    Qt::KeyboardModifier m_tapModifier;

    /*************************************************************************
     * Grand Master
     *************************************************************************/
public:
    /** Set grand master visible/hidden */
    void setGMVisible(bool visible);

    /** Check if grand master slider is visible */
    bool isGMVisible() const;

    void setGrandMasterChannelMode(UniverseArray::GMChannelMode mode);
    UniverseArray::GMChannelMode grandMasterChannelMode() const;

    void setGrandMasterValueMode(UniverseArray::GMValueMode mode);
    UniverseArray::GMValueMode grandMasterValueMode() const;

    void setGrandMasterInputSource(quint32 universe, quint32 channel);
    quint32 grandMasterInputUniverse() const;
    quint32 grandMasterInputChannel() const;

private:
    UniverseArray::GMChannelMode m_gmChannelMode;
    UniverseArray::GMValueMode m_gmValueMode;
    quint32 m_gmInputUniverse;
    quint32 m_gmInputChannel;
    bool m_gmVisible;

    /*************************************************************************
     * Blackout
     *************************************************************************/
public:
    void setBlackoutInputSource(quint32 universe, quint32 channel);
    quint32 blackoutInputUniverse() const;
    quint32 blackoutInputChannel() const;

private:
    quint32 m_blackoutInputUniverse;
    quint32 m_blackoutInputChannel;

    /*************************************************************************
     * Load & Save
     *************************************************************************/
public:
    /** Load VirtualConsole properties from the given XML tag */
    bool loadXML(const QDomElement& vc_root);

    /** Save VirtualConsole properties to the given XML document */
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root) const;

private:
    /** Load the properties of a default slider */
    static bool loadXMLInput(const QDomElement& tag, quint32* universe, quint32* channel);
};

#endif
