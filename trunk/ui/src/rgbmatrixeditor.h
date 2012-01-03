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
class SpeedSpinBox;
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
    void fillAnimationCombo();
    void updateExtraOptions();

    void createPreviewItems();

private slots:
    void slotPreviewTimeout();
    void slotNameEdited(const QString& text);
    void slotPatternActivated(const QString& text);
    void slotFixtureGroupActivated(int index);
    void slotColorButtonClicked();

    void slotTextEdited(const QString& text);
    void slotFontButtonClicked();
    void slotAnimationActivated(const QString& text);
    void slotOffsetSpinChanged();

    void slotLoopClicked();
    void slotPingPongClicked();
    void slotSingleShotClicked();

    void slotForwardClicked();
    void slotBackwardClicked();

    void slotFadeInSpinChanged(int ms);
    void slotFadeOutSpinChanged(int ms);
    void slotDurationSpinChanged(int ms);

    void slotTestClicked();
    void slotRestartTest();

private:
    Doc* m_doc;
    RGBMatrix* m_original;
    RGBMatrix* m_mtx;

    QList <RGBScript> m_scripts;
    QList <RGBMap> m_previewMaps;

    SpeedSpinBox* m_fadeInSpin;
    SpeedSpinBox* m_fadeOutSpin;
    SpeedSpinBox* m_durationSpin;

    QGraphicsScene* m_scene;
    QTimer* m_previewTimer;
    int m_previewIterator;
    int m_previewStep;
    QHash <QLCPoint,QGraphicsItem*> m_previewHash;
};

#endif
