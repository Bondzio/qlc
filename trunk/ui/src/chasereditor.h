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
class Function;
class InputMap;
class OutputMap;
class MasterTimer;
class SpeedSpinBox;
class QTreeWidgetItem;

class ChaserEditor : public QDialog, public Ui_ChaserEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(ChaserEditor)

public:
    ChaserEditor(QWidget* parent, Chaser* chaser, Doc* doc);
    ~ChaserEditor();

private:
    Doc* m_doc;

private:
    void updateFunctionItem(QTreeWidgetItem* item, const Function* function);
    void updateStepNumbers();

private slots:
    void accept();

    void slotNameEdited(const QString& text);

    void slotAddClicked();
    void slotRemoveClicked();
    void slotRaiseClicked();
    void slotLowerClicked();

    void slotCutClicked();
    void slotCopyClicked();
    void slotPasteClicked();

    void slotLoopClicked();
    void slotSingleShotClicked();
    void slotPingPongClicked();

    void slotForwardClicked();
    void slotBackwardClicked();

    void slotFadeInSpinChanged(int ms);
    void slotFadeOutSpinChanged(int ms);
    void slotDurationSpinChanged(int ms);

private:
    Chaser* m_original;
    Chaser* m_chaser;
    QList <quint32> m_clipboard;

    QAction* m_cutAction;
    QAction* m_copyAction;
    QAction* m_pasteAction;

    SpeedSpinBox* m_fadeInSpin;
    SpeedSpinBox* m_fadeOutSpin;
    SpeedSpinBox* m_durationSpin;
};

#endif
