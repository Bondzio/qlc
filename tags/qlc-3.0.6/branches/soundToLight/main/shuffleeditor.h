/*
  Q Light Controller
  shuffleeditor.h

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

#ifndef SHUFFLEEDITOR_H
#define SHUFFLEEDITOR_H

#include <QDialog>
#include "ui_shuffleeditor.h"

class Shuffle;
class FunctionSelection;

class ShuffleEditor : public QDialog, public Ui_ShuffleEditor
{
	Q_OBJECT

public:
	ShuffleEditor(QWidget* parent, Shuffle* shuffle);
	~ShuffleEditor();

private:
	Q_DISABLE_COPY(ShuffleEditor)

protected:
	/** Fill known buses to the bus combo and select current */
	void fillBusCombo();

	/**
	 * Insert chaser steps into the editor's view and select an item
	 * @param selectIndex The index to select
	 */
	void updateStepList(int selectIndex = 0);

	/** Update correct order numbers to each step */
	void updateOrderNumbers();

protected slots:
	void accept();

	/** Name has been edited */
	void slotNameEdited(const QString& text);

	/** Bus has been activated */
	void slotBusComboActivated(int index);

	/** Add a step */
	void slotAddClicked();

	/** Remove the selected step */
	void slotRemoveClicked();

protected:
	/** The copied shuffle that is being edited */
	Shuffle* m_shuffle;

	/** The original shuffle, whose contents will be replaced with the
	    contents of m_shuffle, only if OK is clicked. */
	Shuffle* m_original;
};

#endif
