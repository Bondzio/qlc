/*
  Q Light Controller
  cuestack.h

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

#ifndef CUESTACK_H
#define CUESTACK_H

#include <QObject>
#include <QList>

#include "cue.h"

#define KXMLQLCCueStack "CueStack"
#define KXMLQLCCueStackID "ID"
#define KXMLQLCCueStackSpeed "Speed"
#define KXMLQLCCueStackSpeedFadeIn "FadeIn"
#define KXMLQLCCueStackSpeedFadeOut "FadeOut"
#define KXMLQLCCueStackSpeedDuration "Duration"

class UniverseArray;
class GenericFader;
class QDomDocument;
class QDomElement;
class MasterTimer;
class FadeChannel;
class Doc;

class CueStack : public QObject
{
    Q_OBJECT

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    CueStack(Doc* doc);
    ~CueStack();

private:
    Doc* doc() const;

    /************************************************************************
     * Speed
     ************************************************************************/
public:
    void setFadeInSpeed(uint ms);
    uint fadeInSpeed() const;

    void setFadeOutSpeed(uint ms);
    uint fadeOutSpeed() const;

    void setDuration(uint ms);
    uint duration() const;

private:
    uint m_fadeInSpeed;
    uint m_fadeOutSpeed;
    uint m_duration;

    /************************************************************************
     * Cues
     ************************************************************************/
public:
    /** Append $cue at the end of the cue stack */
    void appendCue(const Cue& cue);

    /** Insert $cue at the given $index */
    void insertCue(int index, const Cue& cue);

    /** Replace the cue at the given $index with $cue */
    void replaceCue(int index, const Cue& cue);

    /** Remove the cue at the given $index */
    void removeCue(int index);

    /** Get a list of all cues */
    QList <Cue> cues() const;

signals:
    void added(int index);
    void removed(int index);
    void changed(int index);

private:
    QList <Cue> m_cues;

    /************************************************************************
     * Load & Save
     ************************************************************************/
public:
    bool loadXML(const QDomElement& root, uint& id);
    bool saveXML(QDomDocument* doc, QDomElement* root, uint id) const;

    /************************************************************************
     * Running
     ************************************************************************/
public:
    void start();
    void stop();
    bool isRunning() const;

    void setCurrentIndex(int index);
    int currentIndex() const;

    void previousCue();
    void nextCue();

    void adjustIntensity(qreal fraction);
    qreal intensity() const;

signals:
    void started();
    void stopped();
    void currentCueChanged(int index);

private:
    bool m_running;
    qreal m_intensity;
    int m_currentIndex;

    /************************************************************************
     * Writing
     ************************************************************************/
public:
    bool isStarted() const;

    void preRun();
    void write(UniverseArray* ua);
    void postRun(MasterTimer* timer);

private:
    int next();
    int previous();
    void switchCue(int index, const UniverseArray* ua);
    void insertStartValue(FadeChannel& fc, const UniverseArray* ua);

private:
    GenericFader* m_fader;
    uint m_elapsed;
    bool m_previous;
    bool m_next;
};

#endif
