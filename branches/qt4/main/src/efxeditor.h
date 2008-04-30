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
#include <QFrame>
#include "common/qlctypes.h"

#include "ui_efxeditor.cpp"
#include "efx.h"

class QPaintEvent;
class EFXPreviewArea;

class EFXEditor : public QDialog, public Ui_EFXEditor
{
	Q_OBJECT

public:
	EFXEditor(QWidget* parent, EFX* efx);
	~EFXEditor();

protected:
	/**
	 * Set the EFX function to edit
	 *
	 * @param efx The EFX function to edit
	 */
	void setEFX(EFX* efx);

	/**
	 * Get channels from the EFX function's fixture
	 * and fill the combos with them.
	 */
	void fillChannelCombos();

	/**
	 * Get sceness from the EFX function's fixture
	 * and fill the list views with them.
	 */
	void fillSceneLists();

protected slots:
	void slotNameEdited(const QString &text);

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

	void slotXAxisActivated(int index);
	void slotYAxisActivated(int index);
  
	void slotStartSceneGroupToggled(bool);
	void slotStopSceneGroupToggled(bool);

	void slotStartSceneListSelectionChanged();
	void slotStopSceneListSelectionChanged();

protected:
	EFXPreviewArea* m_previewArea;
	QPolygon* m_points;

	EFX* m_efx;
};

/**
 * The area that is used to draw a preview of
 * the EFX function currently being edited.
 */
class EFXPreviewArea : public QFrame
{
	Q_OBJECT

public:
	EFXPreviewArea(QWidget* parent);
	~EFXPreviewArea();

	/**
	 * Get the pointer for the point array that is used
	 * to draw the preview
	 *
	 * @return The point array
	 */
	QPolygon* points();

protected:
	/**
	 * QT Framework calls this when the widget needs
	 * to be repainted.
	 *
	 * @param e QPaintEvent
	 */
	void paintEvent(QPaintEvent* e);

protected:
	/** Points that are drawn in the preview area */
	QPolygon* m_points;
};


#endif
