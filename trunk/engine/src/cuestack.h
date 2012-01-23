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
#include <QMutex>
#include <QList>

#include "dmxsource.h"
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

class CueStack : public QObject, public DMXSource
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
     * Name
     ************************************************************************/
public:
    void setName(const QString& name, int index = -1);
    QString name(int index = -1) const;

private:
    QString m_name;

    /************************************************************************
     * Speed
     ************************************************************************/
public:
    void setFadeInSpeed(uint ms, int index = -1);
    uint fadeInSpeed(int index = -1) const;

    void setFadeOutSpeed(uint ms, int index = -1);
    uint fadeOutSpeed(int index = -1) const;

    void setDuration(uint ms, int index = -1);
    uint duration(int index = -1) const;

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
    QMutex m_mutex;

    /************************************************************************
     * Load & Save
     ************************************************************************/
public:
    static uint loadXMLID(const QDomElement& root);
    bool loadXML(const QDomElement& root);
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
     * Flashing
     ************************************************************************/
public:
    void setFlashing(bool enable);
    bool isFlashing() const;

    void writeDMX(MasterTimer* timer, UniverseArray* ua);

private:
    bool m_flashing;

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
    void switchCue(int from, int to, const UniverseArray* ua);
    void insertStartValue(FadeChannel& fc, const UniverseArray* ua);

private:
    GenericFader* m_fader;
    uint m_elapsed;
    bool m_previous;
    bool m_next;
};

#endif
