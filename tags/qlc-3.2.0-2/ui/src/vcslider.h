/*
  Q Light Controller
  vcslider.h

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

#ifndef VCSLIDER_H
#define VCSLIDER_H

#include <QMutex>
#include <QList>

#include "dmxsource.h"
#include "vcwidget.h"

class QDomDocument;
class QDomElement;
class QPushButton;
class QHBoxLayout;
class QSlider;
class QLabel;
class QTime;

class VCSliderProperties;

#define KXMLQLCVCSlider "Slider"
#define KXMLQLCVCSliderMode "SliderMode"

#define KXMLQLCVCSliderValueDisplayStyle "ValueDisplayStyle"
#define KXMLQLCVCSliderValueDisplayStyleExact "Exact"
#define KXMLQLCVCSliderValueDisplayStylePercentage "Percentage"

#define KXMLQLCVCSliderInvertedAppearance "InvertedAppearance"

#define KXMLQLCVCSliderBus "Bus"
#define KXMLQLCVCSliderBusLowLimit "LowLimit"
#define KXMLQLCVCSliderBusHighLimit "HighLimit"

#define KXMLQLCVCSliderLevel "Level"
#define KXMLQLCVCSliderLevelLowLimit "LowLimit"
#define KXMLQLCVCSliderLevelHighLimit "HighLimit"
#define KXMLQLCVCSliderLevelValue "Value"

#define KXMLQLCVCSliderChannel "Channel"
#define KXMLQLCVCSliderChannelFixture "Fixture"

#define KXMLQLCVCSliderPlayback "Playback"
#define KXMLQLCVCSliderPlaybackFunction "Function"

class VCSlider : public VCWidget, public DMXSource
{
    Q_OBJECT
    Q_DISABLE_COPY(VCSlider)

    friend class VCSliderProperties;

public:
    static const QSize defaultSize;

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** Normal constructor */
    VCSlider(QWidget* parent, Doc* doc, OutputMap* outputMap, InputMap* inputMap, MasterTimer* masterTimer);

    /** Destructor */
    ~VCSlider();

    /*********************************************************************
     * Clipboard
     *********************************************************************/
public:
    /** Create a copy of this widget into the given parent */
    VCWidget* createCopy(VCWidget* parent);

protected:
    /** Copy the contents for this widget from another widget */
    bool copyFrom(VCWidget* widget);

    /*********************************************************************
     * Caption
     *********************************************************************/
public:
    void setCaption(const QString& text);

    /*********************************************************************
     * Properties
     *********************************************************************/
public:
    /** Edit this widget's properties */
    void editProperties();

    /*********************************************************************
     * QLC Mode
     *********************************************************************/
public slots:
    void slotModeChanged(Doc::Mode mode);

    /*********************************************************************
     * Slider Mode
     *********************************************************************/
public:
    enum SliderMode
    {
        Bus,
        Level,
        Playback
    };

public:
    /**
     * Convert a SliderMode enum to a string that can be saved into
     * an XML file.
     *
     * @param mode The mode to convert
     * @return A string
     */
    static QString sliderModeToString(SliderMode mode);

    /**
     * Convert a string into a SliderMode enum.
     *
     * @param mode The string to convert
     * @return SliderMode
     */
    static SliderMode stringToSliderMode(const QString& mode);

    /**
     * Get the slider's current SliderMode
     */
    SliderMode sliderMode();

    /**
     * Change the slider's current SliderMode
     */
    void setSliderMode(SliderMode mode);

protected:
    SliderMode m_sliderMode;

    /*********************************************************************
     * Value display style
     *********************************************************************/
public:
    enum ValueDisplayStyle
    {
        ExactValue,
        PercentageValue
    };

    static QString valueDisplayStyleToString(ValueDisplayStyle style);
    static ValueDisplayStyle stringToValueDisplayStyle(QString style);

    void setValueDisplayStyle(ValueDisplayStyle style);
    ValueDisplayStyle valueDisplayStyle();

protected:
    ValueDisplayStyle m_valueDisplayStyle;

    /*********************************************************************
     * Inverted appearance
     *********************************************************************/
public:
    bool invertedAppearance() const;
    void setInvertedAppearance(bool invert);

    /*********************************************************************
     * Bus
     *********************************************************************/
public:
    /**
     * Set the ID of the bus to control (when in Bus mode)
     *
     * @param bus A bus id
     */
    void setBus(quint32 bus);

    /**
     * Get the ID of the controlled bus
     *
     */
    quint32 bus();

    /**
     * Set the low limit for bus values set thru the slider
     */
    void setBusLowLimit(quint32 limit);

    /**
     * Get the low limit for bus values set thru the slider
     */
    quint32 busLowLimit();

    /**
     * Set the high limit for bus values set thru the slider
     */
    void setBusHighLimit(quint32 limit);

    /**
     * Get the high limit for bus values set thru the slider
     */
    quint32 busHighLimit();

protected:
    /**
     * Set the current slider value to the assigned bus
     */
    void setBusValue(int value);

public slots:
    /**
     * Callback for bus value changes
     */
    void slotBusValueChanged(quint32 bus, quint32 value);

    /**
     * Callback for bus name changes
     */
    void slotBusNameChanged(quint32 bus, const QString& name);

protected:
    quint32 m_bus;
    quint32 m_busLowLimit;
    quint32 m_busHighLimit;

    /*************************************************************************
     * Class LevelChannel
     *************************************************************************/
public:
    /**
     * This class is used to store one (fixture, channel) pair that is used by
     * VCSlider to control the level of individual fixture channels.
     */
    class LevelChannel
    {
    public:
        /** Construct a new LevelChannel with the given fixture & channel */
        LevelChannel(quint32 fid, quint32 ch);
        /** Copy constructor */
        LevelChannel(const LevelChannel& lc);
        /** Comparison operator */
        bool operator==(const LevelChannel& lc) const;
        /** Sorting operator */
        bool operator<(const LevelChannel& lc) const;
        /** Save the contents of a LevelChannel instance to an XML document */
        void saveXML(QDomDocument* doc, QDomElement* root) const;

    public:
        /** The associated fixture ID */
        quint32 fixture;
        /** The associated channel within the fixture */
        quint32 channel;
    };

    /*********************************************************************
     * Level channels
     *********************************************************************/
public:
    /**
     * Add a channel from a fixture into the slider's list of
     * level channels.
     *
     * @param fixture Fixture ID
     * @param channel A channel from the fixture
     */
    void addLevelChannel(quint32 fixture, quint32 channel);

    /**
     * Remove a fixture & channel from the slider's list of
     * level channels.
     *
     * @param fixture Fixture ID
     * @param channel A channel from the fixture
     */
    void removeLevelChannel(quint32 fixture, quint32 channel);

    /**
     * Clear the list of level channels
     *
     */
    void clearLevelChannels();

    /**
     * Get the list of channels that this slider controls
     *
     */
    QList <VCSlider::LevelChannel> levelChannels();

    /**
     * Set low limit for levels set thru the slider
     *
     * @param value Low limit
     */
    void setLevelLowLimit(uchar value);

    /**
     * Get low limit for levels set thru the slider
     *
     */
    uchar levelLowLimit();

    /**
     * Set high limit for levels set thru the slider
     *
     * @param value High limit
     */
    void setLevelHighLimit(uchar value);

    /**
     * Get high limit for levels set thru the slider
     *
     */
    uchar levelHighLimit();

protected:
    /**
     * Set the level to all channels that have been assigned to
     * the slider.
     *
     * @param value DMX value
     */
    void setLevelValue(uchar value);

    /**
     * Get the current "level" mode value
     */
    uchar levelValue() const;

protected slots:
    /** Removes all level channels related to removed fixture */
    void slotFixtureRemoved(quint32 fxi_id);

protected:
    QList <VCSlider::LevelChannel> m_levelChannels;
    uchar m_levelLowLimit;
    uchar m_levelHighLimit;

    QMutex m_levelValueMutex;
    bool m_levelValueChanged;
    uchar m_levelValue;

    /*********************************************************************
     * Playback
     *********************************************************************/
public:
    /**
     * Set the function used as the slider's playback function (when in
     * playback mode).
     *
     * @param fid The ID of the function
     */
    void setPlaybackFunction(quint32 fid);

    /**
     * Get the function used as the slider's playback function (when in
     * playback mode).
     *
     * @return The ID of the function
     */
    quint32 playbackFunction() const;

    /**
     * Set the level of the currently selected playback function.
     *
     * @param level The current playback function's level.
     */
    void setPlaybackValue(uchar value);

    /**
     * Get the level of the currently selected playback function.
     *
     * @return The current playback function level.
     */
    uchar playbackValue() const;

protected slots:
    void slotPlaybackFunctionRunning(quint32 fid);
    void slotPlaybackFunctionStopped(quint32 fid);
    void slotPlaybackFunctionIntensityChanged(qreal fraction);

protected:
    quint32 m_playbackFunction;
    uchar m_playbackValue;
    bool m_playbackValueChanged;
    QMutex m_playbackValueMutex;

    /*********************************************************************
     * DMXSource
     *********************************************************************/
public:
    /** @reimpl */
    void writeDMX(MasterTimer* timer, UniverseArray* universes);

protected:
    /** writeDMX for Level mode */
    void writeDMXLevel(MasterTimer* timer, UniverseArray* universes);

    /** writeDMX for Playback mode */
    void writeDMXPlayback(MasterTimer* timer, UniverseArray* universes);

    /*********************************************************************
     * Top label
     *********************************************************************/
public:
    /**
     * Set the text for the top label
     */
    void setTopLabelText(const QString& text);

    /**
     * Get the text in the top label
     */
    QString topLabelText();

protected:
    QLabel* m_topLabel;

    /*********************************************************************
     * Slider
     *********************************************************************/
public:
    int sliderValue() const;

private slots:
    void slotSliderMoved(int value);

private:
    void sendFeedBack(int value);

protected:
    QHBoxLayout* m_hbox;
    QSlider* m_slider;
    bool m_externalMovement;

    /*********************************************************************
     * Bottom label
     *********************************************************************/
public:
    /**
     * Set the text for the bottom label
     */
    void setBottomLabelText(const QString& text);

    /**
     * Get the text in the top label
     */
    QString bottomLabelText();

protected:
    QLabel* m_bottomLabel;

    /*********************************************************************
     * Tap button
     *********************************************************************/
public:
    /**
     * Set the text for the tap button
     */
    void setTapButtonText(const QString& text);

    /**
     * Get the text in the tap button
     */
    QString tapButtonText();

public slots:
    /**
     * Callback for tap button clicks
     */
    void slotTapButtonClicked();

protected:
    QPushButton* m_tapButton;
    QTime* m_time;

    /*********************************************************************
     * External input
     *********************************************************************/

protected:
    /**
     * Check, whether the given channel's type is QLCInputProfile::Button.
     * If no input profile has been assigned to the universe, this will
     * always return false.
     *
     * @return true if the channel represents a button, otherwise false
     */
    bool isButton(quint32 universe, quint32 channel);

protected slots:
    /** Called when an external input device produces input data */
    void slotInputValueChanged(quint32 universe, quint32 channel,
                               uchar value);

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    bool loadXML(const QDomElement* root);
    bool loadXMLLevel(const QDomElement* level_root);
    bool loadXMLPlayback(const QDomElement* pb_root);

    bool saveXML(QDomDocument* doc, QDomElement* vc_root);
};

#endif