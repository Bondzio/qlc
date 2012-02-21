/*
  Q Light Controller
  sceneeditor.h

  Copyright (c) Heikki Junnila, Stefan Krumm

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

#ifndef SCENEEDITOR_H
#define SCENEEDITOR_H

#include <QWidget>
#include <QList>
#include <QMap>

#include "ui_sceneeditor.h"
#include "fixture.h"
#include "scene.h"

class GenericDMXSource;
class SpeedDialWidget;
class MasterTimer;
class OutputMap;
class InputMap;
class QAction;
class Doc;

class SceneEditor : public QWidget, public Ui_SceneEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(SceneEditor)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    SceneEditor(QWidget* parent, Scene* scene, Doc* doc);
    ~SceneEditor();

private:
    Doc* m_doc;
    Scene* m_scene; // The Scene that is being edited
    GenericDMXSource* m_source;

private:
    void init();
    void setSceneValue(const SceneValue& scv);

private:
    bool m_initFinished;

    /*********************************************************************
     * Common
     *********************************************************************/
private slots:
    void slotTabChanged(int tab);

    void slotEnableCurrent();
    void slotDisableCurrent();

    void slotCopy();
    void slotPaste();
    void slotCopyToAll();
    void slotColorTool();
    void slotBlindToggled(bool state);
    void slotModeChanged(Doc::Mode mode);

private:
    bool isColorToolAvailable();

private:
    QAction* m_enableCurrentAction;
    QAction* m_disableCurrentAction;

    QAction* m_copyAction;
    QAction* m_pasteAction;
    QAction* m_copyToAllAction;
    QAction* m_colorToolAction;

    QAction* m_blindAction;

    /*********************************************************************
     * General tab
     *********************************************************************/
private:
    QTreeWidgetItem* fixtureItem(quint32 fxi_id);
    QList <Fixture*> selectedFixtures() const;

    void addFixtureItem(Fixture* fixture);
    void removeFixtureItem(Fixture* fixture);

private slots:
    void slotNameEdited(const QString& name);
    void slotAddFixtureClicked();
    void slotRemoveFixtureClicked();

    void slotEnableAll();
    void slotDisableAll();

    void slotFadeInChanged(int ms);
    void slotFadeOutChanged(int ms);

private:
    SpeedDialWidget* m_speedDials;

    /*********************************************************************
     * Fixture tabs
     *********************************************************************/
private:
    FixtureConsole* fixtureConsole(Fixture* fixture);

    void addFixtureTab(Fixture* fixture);
    void removeFixtureTab(Fixture* fixture);
    FixtureConsole* consoleTab(int tab);

private slots:
    void slotValueChanged(quint32 fxi, quint32 channel, uchar value);
    void slotChecked(quint32 fxi, quint32 channel, bool state);

private:
    int m_currentTab;

    /** Values from a copied console (i.e. clipboard) */
    QList <SceneValue> m_copy;
};

#endif
