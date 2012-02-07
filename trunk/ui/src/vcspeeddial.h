/*
  Q Light Controller
  vcspeeddial.h

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

#ifndef VCSPEEDDIAL_H
#define VCSPEEDDIAL_H

#include <QSet>

#include "vcwidget.h"

#define KXMLQLCVCSpeedDial "SpeedDial"

#define KXMLQLCVCSpeedDialFunction "Function"

#define KXMLQLCVCSpeedDialSpeedType "SpeedType"
#define KXMLQLCVCSpeedDialSpeedTypeFadeIn "FadeIn"
#define KXMLQLCVCSpeedDialSpeedTypeFadeOut "FadeOut"
#define KXMLQLCVCSpeedDialSpeedTypeDuration "Duration"

class QDomDocument;
class QDomElement;
class SpeedDial;

class VCSpeedDial : public VCWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(VCSpeedDial)

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    VCSpeedDial(QWidget* parent, Doc* doc);
    ~VCSpeedDial();

    /*************************************************************************
     * Clipboard
     *************************************************************************/
public:
    /** @reimp */
    VCWidget* createCopy(VCWidget* parent);

protected:
    /** @reimp */
    bool copyFrom(VCWidget* widget);

    /*************************************************************************
     * Caption
     *************************************************************************/
public:
    /** @reimp */
    void setCaption(const QString& text);

    /*************************************************************************
     * QLC Mode
     *************************************************************************/
public slots:
    /** @reimp */
    void slotModeChanged(Doc::Mode mode);

    /************************************************************************
     * Speed Type
     ************************************************************************/
public:
    enum SpeedType { FadeIn, FadeOut, Duration };

    /**
     * Set the speed type that is to be controlled thru the dial. See
     * enum SpeedType for possible values.
     *
     * @param type The Speed type to control
     */
    void setSpeedType(SpeedType type);

    /**
     * Get the speed type that is controlled thru the dial.
     *
     * @return The speed type controlled by the dial
     */
    SpeedType speedType() const;

    /** Convert the given string into an enum SpeedType */
    static VCSpeedDial::SpeedType stringToSpeedType(const QString& str);

    /** Convert the given SpeedType into a string */
    static QString speedTypeToString(VCSpeedDial::SpeedType type);

private:
    SpeedType m_speedType;

    /************************************************************************
     * Functions
     ************************************************************************/
public:
    /**
     * Add a function, whose speed is controlled by the dial.
     *
     * @param id The ID of the function to control
     */
    void addFunction(quint32 id);

    /**
     * Remove a function from the dial's controlled functions.
     *
     * @param id The ID of the function to remove
     */
    void removeFunction(quint32 id);

    /**
     * Get the set of functions that are controlled by the slider.
     */
    QSet <quint32> functions() const;

private slots:
    /** Catch dial value changes and patch them to controlled functions */
    void slotDialValueChanged(uint ms);

private:
    QSet <quint32> m_functions;
    SpeedDial* m_dial;

    /*************************************************************************
     * Load & Save
     *************************************************************************/
public:
    /** @reimp */
    bool loadXML(const QDomElement* root);

    /** @reimp */
    bool saveXML(QDomDocument* doc, QDomElement* vc_root);

    /** @reimp */
    void postLoad();
};

#endif
