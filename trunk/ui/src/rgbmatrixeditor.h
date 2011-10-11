/*
  Q Light Controller
  rgbmatrixeditor.h

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

#ifndef RGBMATRIXEDITOR_H
#define RGBMATRIXEDITOR_H

#include <QDialog>
#include <QHash>

#include "ui_rgbmatrixeditor.h"
#include "rgbmatrix.h"
#include "qlcpoint.h"

class QGraphicsScene;
class RGBMatrix;
class QTimer;
class Doc;

class RGBMatrixEditor : public QDialog, public Ui_RGBMatrixEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(RGBMatrixEditor)

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    RGBMatrixEditor(QWidget* parent, RGBMatrix* mtx, Doc* doc);
    ~RGBMatrixEditor();

public slots:
    void accept();

private:
    void init();

    void fillPatternCombo();
    void fillFixtureGroupCombo();

    void createPreviewItems();

private slots:
    void slotPreviewTimeout();
    void slotNameEdited(const QString& text);
    void slotPatternActivated(const QString& text);
    void slotFixtureGroupActivated(int index);
    void slotColorButtonClicked();

    void slotLoopClicked();
    void slotPingPongClicked();
    void slotSingleShotClicked();

    void slotForwardClicked();
    void slotBackwardClicked();

    void slotFadeInSpinChanged(double seconds);
    void slotFadeOutSpinChanged(double seconds);
    void slotPatternSpinChanged(double seconds);

    void slotTestClicked();
    void slotRestartTest();

private:
    Doc* m_doc;
    RGBMatrix* m_original;
    RGBMatrix* m_mtx;

    QGraphicsScene* m_scene;
    QTimer* m_previewTimer;
    int m_previewIterator;
    QHash <QLCPoint,QGraphicsItem*> m_previewHash;
};

#endif
