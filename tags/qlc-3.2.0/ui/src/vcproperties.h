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

#include <QDialog>

#include "vcwidgetproperties.h"
#include "ui_vcproperties.h"
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

#define KXMLQLCVCPropertiesDefaultSlider "DefaultSlider"
#define KXMLQLCVCPropertiesDefaultSliderRole "Role"
#define KXMLQLCVCPropertiesDefaultSliderRoleFade "Fade"
#define KXMLQLCVCPropertiesDefaultSliderRoleHold "Hold"
#define KXMLQLCVCPropertiesDefaultSliderVisible "Visible"
#define KXMLQLCVCPropertiesLowLimit "Low"
#define KXMLQLCVCPropertiesHighLimit "High"

#define KXMLQLCVCPropertiesGrandMaster "GrandMaster"
#define KXMLQLCVCPropertiesGrandMasterChannelMode "ChannelMode"
#define KXMLQLCVCPropertiesGrandMasterValueMode "ValueMode"

#define KXMLQLCVCPropertiesBlackout "Blackout"

#define KXMLQLCVCPropertiesInput "Input"
#define KXMLQLCVCPropertiesInputUniverse "Universe"
#define KXMLQLCVCPropertiesInputChannel "Channel"

/*****************************************************************************
 * Properties
 *****************************************************************************/

class VCProperties : public VCWidgetProperties
{
public:
    VCProperties();
    VCProperties(const VCProperties& properties);
    ~VCProperties();

    VCProperties& operator=(const VCProperties& properties);

    /*********************************************************************
     * VC Contents
     *********************************************************************/
public:
    /** Get Virtual Console's bottom-most frame */
    VCFrame* contents() const;

    /** Reset Virtual Console's bottom-most frame to initial state */
    void resetContents(QWidget* parent, Doc* doc, OutputMap* outputMap,
                       InputMap* inputMap, MasterTimer* masterTimer);

private:
    VCFrame* m_contents;

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
     * Keyboard state
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

private:
    bool m_keyRepeatOff;
    bool m_grabKeyboard;

    /*************************************************************************
     * Grand Master
     *************************************************************************/
public:
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

    /*********************************************************************
     * Default Fade Slider
     *********************************************************************/
public:
    /** Set, whether default sliders are visible */
    void setSlidersVisible(bool visible);

    /** Check if default sliders are visible */
    bool slidersVisible() const;

    /** Set limits for fade slider */
    void setFadeLimits(quint32 low, quint32 high);
    quint32 fadeLowLimit() const;
    quint32 fadeHighLimit() const;

    /** Set input source for fade slider */
    void setFadeInputSource(quint32 uni, quint32 ch);
    quint32 fadeInputUniverse() const;
    quint32 fadeInputChannel() const;

private:
    bool m_slidersVisible;

    quint32 m_fadeLowLimit;
    quint32 m_fadeHighLimit;
    quint32 m_fadeInputUniverse;
    quint32 m_fadeInputChannel;

    /*********************************************************************
     * Default Hold Slider
     *********************************************************************/
public:
    /** Set limits for hold slider */
    void setHoldLimits(quint32 low, quint32 high);
    quint32 holdLowLimit() const;
    quint32 holdHighLimit() const;

    /** Set input source for hold slider */
    void setHoldInputSource(quint32 uni, quint32 ch);
    quint32 holdInputUniverse() const;
    quint32 holdInputChannel() const;

private:
    quint32 m_holdLowLimit;
    quint32 m_holdHighLimit;
    quint32 m_holdInputUniverse;
    quint32 m_holdInputChannel;

    /*************************************************************************
     * Load & Save
     *************************************************************************/
public:
    /** Load VirtualConsole properties & contents from the given XML tag */
    bool loadXML(const QDomElement& vc_root);

    /** Save VirtualConsole properties & contents to the given XML document */
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

private:
    /** Load VirtualConsole properties (not including contents) */
    bool loadProperties(const QDomElement& root);

    /** Load the properties of a default slider */
    static bool loadXMLInput(const QDomElement& tag, quint32* universe, quint32* channel);
};

#endif
