/*
  Q Light Controller
  efxeditor.h

  Copyright (C) Heikki Junnila

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

#ifndef EFXEDITOR_H
#define EFXEDITOR_H

#include <QPolygon>
#include <QWidget>
#include <QFrame>
#include <QTimer>

#include "ui_efxeditor.h"
#include "efx.h"
#include "doc.h"

class EFXPreviewArea;
class SpeedSpinBox;
class Doc;

class EFXEditor : public QWidget, public Ui_EFXEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(EFXEditor)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    EFXEditor(QWidget* parent, EFX* efx, Doc* doc);
    ~EFXEditor();

private:
    Doc* m_doc;
    EFX* m_efx; // The EFX being edited

private:
    void initGeneralPage();
    void initMovementPage();
    void initInitializationPage();

private slots:
    void slotTestClicked();
    void slotRestartTest();
    void slotModeChanged(Doc::Mode mode);

private:
    EFXPreviewArea* m_previewArea;
    QPolygon* m_points;
    QTimer m_testTimer;

    /*********************************************************************
     * General page
     *********************************************************************/
private:
    QTreeWidgetItem* fixtureItem(EFXFixture* ef);
    const QList <EFXFixture*> selectedFixtures() const;
    void updateIndices(int from, int to);
    void addFixtureItem(EFXFixture* ef);
    void updateIntensityColumn(QTreeWidgetItem* item, EFXFixture* ef);
    void removeFixtureItem(EFXFixture* ef);

private slots:
    void slotNameEdited(const QString &text);
    void slotFixtureItemChanged(QTreeWidgetItem* item, int column);
    void slotFixtureIntensityChanged(int intensity);
    void slotAddFixtureClicked();
    void slotRemoveFixtureClicked();
    void slotRaiseFixtureClicked();
    void slotLowerFixtureClicked();

    void slotParallelRadioToggled(bool state);
    void slotSerialRadioToggled(bool state);
    void slotAsymmetricRadioToggled(bool state);

    void slotFadeInSpinChanged(int ms);
    void slotFadeOutSpinChanged(int ms);
    void slotDurationSpinChanged(int ms);

private:
    SpeedSpinBox* m_fadeInSpin;
    SpeedSpinBox* m_fadeOutSpin;
    SpeedSpinBox* m_durationSpin;

    /*********************************************************************
     * Movement page
     *********************************************************************/
private slots:
    void slotAlgorithmSelected(const QString &text);
    void slotWidthSpinChanged(int value);
    void slotHeightSpinChanged(int value);
    void slotXOffsetSpinChanged(int value);
    void slotYOffsetSpinChanged(int value);
    void slotRotationSpinChanged(int value);

    void slotXFrequencySpinChanged(int value);
    void slotYFrequencySpinChanged(int value);
    void slotXPhaseSpinChanged(int value);
    void slotYPhaseSpinChanged(int value);

    void slotLoopClicked();
    void slotSingleShotClicked();
    void slotPingPongClicked();

    void slotForwardClicked();
    void slotBackwardClicked();

private:
    void redrawPreview();
};

#endif
