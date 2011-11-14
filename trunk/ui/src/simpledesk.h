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

#include <QWidget>
#include <QList>
#include <QHash>

#include "ui_simpledesk.h"

class GrandMasterSlider;
class SimpleDeskEngine;
class SimpleDesk;
class DMXSlider;
class Doc;

class SimpleDesk : public QWidget, public Ui_SimpleDesk
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** Get the SimpleDesk singleton instance. Can be NULL. */
    static SimpleDesk* instance();

    /** Create and/or show SimpleDesk */
    static void createAndShow(QWidget* parent, Doc* doc);

    /** Normal public destructor */
    ~SimpleDesk();

private:
    /** Private constructor to prevent multiple instances. */
    SimpleDesk(QWidget* parent, Doc* doc);

private:
    static SimpleDesk* s_instance;
    SimpleDeskEngine* m_engine;
    Doc* m_doc;

    /*********************************************************************
     * Universe controls
     *********************************************************************/
private:
    void initUniverseSliders();
    void initUniversePager();

private slots:
    void slotUniversePageUpClicked();
    void slotUniversePageDownClicked();
    void slotUniversePageChanged(int page);
    void slotUniverseSliderValueChanged(uchar value);
    void slotUpdateUniverseSliders();

private:
    QList <DMXSlider*> m_universeSliders;

    /*********************************************************************
     * Grand Master
     *********************************************************************/
private:
    void initGrandMaster();

private:
    GrandMasterSlider* m_grandMasterSlider;
};

#endif
