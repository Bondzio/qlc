/*
  Q Light Controller
  chasereditor.h

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

#ifndef CHASEREDITOR_H
#define CHASEREDITOR_H

#include <QDialog>
#include "ui_chasereditor.h"

class Doc;
class Chaser;
class InputMap;
class OutputMap;
class MasterTimer;

class ChaserEditor : public QDialog, public Ui_ChaserEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(ChaserEditor)

public:
    ChaserEditor(QWidget* parent, Chaser* chaser, Doc* doc, OutputMap* outputMap,
                 InputMap* inputMap, MasterTimer* masterTimer);
    ~ChaserEditor();

private:
    Doc* m_doc;
    OutputMap* m_outputMap;
    InputMap* m_inputMap;
    MasterTimer* m_masterTimer;

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

    /** Raise the selected step */
    void slotRaiseClicked();

    /** Lower the selected step */
    void slotLowerClicked();

protected:
    /** The copied chaser that is being edited */
    Chaser* m_chaser;

    /** The original chaser, whose contents will be replaced with the
        contents of m_chaser, only if OK is clicked. */
    Chaser* m_original;
};

#endif
