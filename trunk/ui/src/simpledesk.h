/*
  Q Light Controller
  simpledesk.h

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

#ifndef SIMPLEDESK_H
#define SIMPLEDESK_H

#include <QModelIndex>
#include <QPointer>
#include <QWidget>
#include <QList>
#include <QHash>

#define KXMLQLCSimpleDesk "SimpleDesk"

class GrandMasterSlider;
class SimpleDeskEngine;
class SpeedDialWidget;
class PlaybackSlider;
class QDomDocument;
class QDomElement;
class QToolButton;
class SimpleDesk;
class QGroupBox;
class QTreeView;
class QSplitter;
class DMXSlider;
class QSpinBox;
class CueStack;
class Doc;
class Cue;

class SimpleDesk : public QWidget
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    SimpleDesk(QWidget* parent, Doc* doc);
    ~SimpleDesk();

    /** Get the singleton instance */
    static SimpleDesk* instance();

    /** Start from scratch; clear everything */
    void clearContents();

private:
    /** Private constructor to prevent multiple instances. */

    /** Initialize the simple desk engine */
    void initEngine();

    /** Initialize the simple desk view components */
    void initView();
    void initLeftSide();
    void initRightSide();

private:
    static SimpleDesk* s_instance;
    SimpleDeskEngine* m_engine;
    QSplitter* m_splitter;
    Doc* m_doc;

    /*********************************************************************
     * Universe controls
     *********************************************************************/
private:
    void initUniverseSliders();
    void initUniversePager();
    void resetUniverseSliders();
    void setChannelName(DMXSlider* slider, uint absch);

private slots:
    void slotUniversePageUpClicked();
    void slotUniversePageDownClicked();
    void slotUniversePageChanged(int page);
    void slotUniverseResetClicked();
    void slotUniverseSliderValueChanged(uchar value);
    void slotUpdateUniverseSliders();

private:
    QGroupBox* m_universeGroup;
    QToolButton* m_universePageUpButton;
    QSpinBox* m_universePageSpin;
    QToolButton* m_universePageDownButton;
    QToolButton* m_universeResetButton;
    GrandMasterSlider* m_grandMasterSlider;

    QList <DMXSlider*> m_universeSliders;
    uint m_channelsPerPage;
    bool m_showChannelNames;

    /*********************************************************************
     * Playback sliders
     *********************************************************************/
private:
    void initPlaybackSliders();
    void resetPlaybackSliders();

private slots:
    void slotPlaybackSelected();
    void slotSelectPlayback(uint pb);
    void slotPlaybackStarted();
    void slotPlaybackStopped();
    void slotPlaybackFlashing(bool enabled);
    void slotPlaybackValueChanged(uchar value);

private:
    QGroupBox* m_playbackGroup;
    QList <PlaybackSlider*> m_playbackSliders;
    uint m_selectedPlayback;
    uint m_playbacksPerPage;

    /*********************************************************************
     * Cue Stack controls
     *********************************************************************/
private:
    void initCueStack();
    void updateCueStackButtons();
    void replaceCurrentCue();
    void updateSpeedDials();

    CueStack* currentCueStack() const;
    int currentCueIndex() const;

private slots:
    void slotCueStackStarted(uint stack);
    void slotCueStackStopped(uint stack);
    void slotCueStackSelectionChanged();

    void slotPreviousCueClicked();
    void slotNextCueClicked();
    void slotStopCueStackClicked();
    void slotCloneCueStackClicked();
    void slotEditCueStackClicked();
    void slotRecordCueClicked();
    void slotDeleteCueClicked();

    void slotFadeInDialChanged(int ms);
    void slotFadeOutDialChanged(int ms);
    void slotDurationDialChanged(int ms);
    void slotCueNameEdited(const QString& name);

protected:
    /** @reimp */
    void showEvent(QShowEvent* ev);

    /** @reimp */
    void hideEvent(QHideEvent* ev);

private:
    QGroupBox* m_cueStackGroup;
    QToolButton* m_previousCueButton;
    QToolButton* m_nextCueButton;
    QToolButton* m_stopCueStackButton;
    QToolButton* m_cloneCueStackButton;
    QToolButton* m_editCueStackButton;
    QToolButton* m_recordCueButton;
    QTreeView* m_cueStackView;
    QPointer<SpeedDialWidget> m_speedDials;
    QModelIndex m_cueDeleteIconIndex;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    bool loadXML(const QDomElement& root);
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root) const;
};

#endif
