/*
  Q Light Controller
  efxeditor.cpp
  
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

#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QPaintEvent>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QPainter>
#include <QLabel>
#include <QPen>

#include "common/qlcfixturedef.h"
#include "common/qlcchannel.h"

#include "efxeditor.h"
#include "fixture.h"
#include "app.h"
#include "doc.h"

extern App* _app;

EFXEditor::EFXEditor(QWidget* parent, EFX* efx) : QDialog(parent)
{
	Q_ASSERT(efx != NULL);

	setupUi(this);

	m_previewArea = new EFXPreviewArea(m_previewFrame);

	/* General page UI connections */
	connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
		this, SLOT(slotNameEdited(const QString&)));

	connect(m_xCombo, SIGNAL(activated(int)),
		this, SLOT(slotXAxisActivated(int)));
	connect(m_yCombo, SIGNAL(activated(int)),
		this, SLOT(slotYAxisActivated(int)));

	connect(m_algorithmCombo, SIGNAL(activated(const QString&)),
		this, SLOT(slotAlgorithmSelected(const QString&)));
	connect(m_widthSpin, SIGNAL(valueChanged(int)),
		this, SLOT(slotWidthSpinChanged(int)));
	connect(m_heightSpin, SIGNAL(valueChanged(int)),
		this, SLOT(slotHeightSpinChanged(int)));
	connect(m_xOffsetSpin, SIGNAL(valueChanged(int)),
		this, SLOT(slotXOffsetSpinChanged(int)));
	connect(m_yOffsetSpin, SIGNAL(valueChanged(int)),
		this, SLOT(slotYOffsetSpinChanged(int)));
	connect(m_rotationSpin, SIGNAL(valueChanged(int)),
		this, SLOT(slotRotationSpinChanged(int)));

	connect(m_xFrequencySpin, SIGNAL(valueChanged(int)),
		this, SLOT(slotXFrequencySpinChanged(int)));
	connect(m_yFrequencySpin, SIGNAL(valueChanged(int)),
		this, SLOT(slotYFrequencySpinChanged(int)));
	connect(m_xPhaseSpin, SIGNAL(valueChanged(int)),
		this, SLOT(slotXPhaseSpinChanged(int)));
	connect(m_yPhaseSpin, SIGNAL(valueChanged(int)),
		this, SLOT(slotYPhaseSpinChanged(int)));

	/* Advanced page UI connections */
	connect(m_loop, SIGNAL(clicked()),
		this, SLOT(slotLoopClicked()));
	connect(m_singleShot, SIGNAL(clicked()),
		this, SLOT(slotSingleShotClicked()));
	connect(m_pingPong, SIGNAL(clicked()),
		this, SLOT(slotPingPongClicked()));

	connect(m_forward, SIGNAL(clicked()),
		this, SLOT(slotForwardClicked()));
	connect(m_backward, SIGNAL(clicked()),
		this, SLOT(slotBackwardClicked()));

	connect(m_startSceneGroup, SIGNAL(toggled(bool)),
		this, SLOT(slotStartSceneGroupToggled(bool)));
	connect(m_stopSceneGroup, SIGNAL(toggled(bool)),
		this, SLOT(slotStopSceneGroupToggled(bool)));	
	connect(m_startSceneList, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotStartSceneListSelectionChanged()));
	connect(m_stopSceneList, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotStopSceneListSelectionChanged()));

	/* Get supported algorithms and fill the algorithm combo with them */
	QStringList list;
	EFX::algorithmList(list);
	m_algorithmCombo->addItems(list);

	/* Resize the preview area to fill its frame */
	m_previewArea->resize(m_previewFrame->width(),
			      m_previewFrame->height());

	/* Set the currently edited EFX function */
	setEFX(efx);

	/* Draw the points */
	m_previewArea->repaint();
}

EFXEditor::~EFXEditor()
{
	m_efx->setPreviewPointArray(NULL);
}

void EFXEditor::setEFX(EFX* efx)
{
	/* Take the new EFX function for editing */
	m_efx = efx;

	/* Set the algorithm's name to the name field */
	m_nameEdit->setText(m_efx->name());
	slotNameEdited(m_efx->name());

	/* Causes the EFX function to update the preview point array */
	slotAlgorithmSelected(m_efx->algorithm());

	/* Set the preview point array for the new EFX */
	m_efx->setPreviewPointArray(m_previewArea->points());

	/* Select the EFX's algorithm from the algorithm combo */
	for (int i = 0; i < m_algorithmCombo->count(); i++)
	{
		if (m_algorithmCombo->itemText(i) == m_efx->algorithm())
		{
			m_algorithmCombo->setCurrentIndex(i);
			break;
		}
	}

	/* Get the algorithm parameters */
	m_widthSpin->setValue(m_efx->width());
	m_heightSpin->setValue(m_efx->height());
	m_xOffsetSpin->setValue(m_efx->xOffset());
	m_yOffsetSpin->setValue(m_efx->yOffset());
	m_rotationSpin->setValue(m_efx->rotation());
  
	m_xFrequencySpin->setValue(m_efx->xFrequency());
	m_yFrequencySpin->setValue(m_efx->yFrequency());
	m_xPhaseSpin->setValue(m_efx->xPhase());
	m_yPhaseSpin->setValue(m_efx->yPhase());

	/* Get advanced parameters */
	switch (m_efx->runOrder())
	{
	default:
	case Function::Loop:
		m_loop->setChecked(true);
		break;
	case Function::SingleShot:
		m_singleShot->setChecked(true);
		break;
	case Function::PingPong:
		m_pingPong->setChecked(true);
		break;
	}

	switch (m_efx->direction())
	{
	default:
	case Function::Forward:
		m_forward->setChecked(true);
		break;
	case Function::Backward:
		m_backward->setChecked(true);
		break;
	}

	fillChannelCombos();
	fillSceneLists();
}

void EFXEditor::fillChannelCombos()
{
	QLCChannel* ch = NULL;
	Fixture* fxi = NULL;
	QString s;

	fxi = _app->doc()->fixture(m_efx->fixture());
	Q_ASSERT(fxi != NULL);

	for (t_channel i = 0; i < fxi->channels(); i++)
	{
		ch = fxi->channel(i);
		if (ch != NULL)
		{
			// Insert ch:name strings to combos for
			// normal fixtures
			s = QString("%1: %2").arg(i + 1).arg(ch->name());
			m_xCombo->addItem(s);
			m_yCombo->addItem(s);
		}
		else
		{
			// Insert ch:Level strings to combos
			// for generic dimmer fixtures
			s = QString("%1: Level").arg(i + 1);
			m_xCombo->addItem(s);
			m_yCombo->addItem(s);
		}
	}

	/* Select a channel as the X axis */
	if (m_efx->xChannel() != KChannelInvalid)
	{
		/* If the EFX already has a valid x channel, select it instead */
		m_xCombo->setCurrentIndex(m_efx->xChannel());
	}
	else if (fxi->fixtureDef() != NULL && fxi->fixtureMode() != NULL)
	{
		/* Try to select a "pan" channel as the Y axis for
		   normal fixtures */
		for (t_channel i = 0; i < fxi->channels(); i++)
		{
			ch = fxi->channel(i);
      
			// Select the first channel that contains "pan"
			if (ch->name().contains("pan", Qt::CaseInsensitive))
			{
				m_xCombo->setCurrentIndex(i);
				m_efx->setXChannel(i);
				break;
			}
		}
	}
	else
	{
		m_xCombo->setCurrentIndex(0);
		m_efx->setXChannel(0);
	}

	/* Select a channel as the X axis */
	if (m_efx->yChannel() != KChannelInvalid)
	{
		/* If the EFX already has valid y channel, select it instead */
		m_yCombo->setCurrentIndex(m_efx->yChannel());
	}
	else if (fxi->fixtureDef() != NULL && fxi->fixtureMode() != NULL)
	{
		/* Try to select a "tilt" channel as the Y axis for
		   normal fixtures */
		for (t_channel i = 0; i < fxi->channels(); i++)
		{
			QLCChannel* ch = fxi->channel(i);
			Q_ASSERT(ch != NULL);
      
			// Select the first channel that contains "tilt"
			if (ch->name().contains("tilt", Qt::CaseInsensitive))
			{
				m_yCombo->setCurrentIndex(i);
				m_efx->setYChannel(i);
				break;
			}
		}
	}
	else
	{
		m_yCombo->setCurrentIndex(0);
		m_efx->setXChannel(0);
	}
}

void EFXEditor::fillSceneLists()
{
	Function* function;
	QTreeWidgetItem* item;
	QTreeWidgetItem* startItem = NULL;
	QTreeWidgetItem* stopItem = NULL;
	QString s;

	Q_ASSERT(m_efx != NULL);
  
	for (t_function_id id = 0; id < KFunctionArraySize; id++)
	{
		function = _app->doc()->function(id);
		if (function == NULL)
			continue;
	
		if (function->type() == Function::Scene && 
		    function->fixture() == m_efx->fixture())
		{
			/* Insert the function to start scene list */
			item = new QTreeWidgetItem(m_startSceneList);
			item->setText(0, function->name());
			item->setText(1, s.setNum(function->id()));

			/* Select the scene from the start scene list */
			if (m_efx->startScene() == function->id())
			{
				m_startSceneList->setCurrentItem(item);
				startItem = item;
			}

			/* Insert the function to stop scene list */
			item = new QTreeWidgetItem(m_stopSceneList);
			item->setText(0, function->name());
			item->setText(1, s.setNum(function->id()));

			/* Select the scene from the stop scene list */
			if (m_efx->stopScene() == function->id())
			{
				m_stopSceneList->setCurrentItem(item);
				stopItem = item;
			}
		}
	}
  
	/* Make sure that the selected start scene item is visible */
	if (startItem != NULL)
		m_startSceneList->scrollToItem(startItem);

	/* Make sure that the selected stop scene item is visible */
	if (stopItem != NULL)
		m_stopSceneList->scrollToItem(stopItem);

	/* Start scene enabled status */
	if (m_efx->startSceneEnabled() == true)
		m_startSceneGroup->setChecked(true);
	else
		m_startSceneGroup->setChecked(false);

	/* Stop scene enabled status */
	if (m_efx->stopSceneEnabled() == true)
		m_stopSceneGroup->setChecked(true);
	else
		m_stopSceneGroup->setChecked(false);
}

void EFXEditor::slotNameEdited(const QString &text)
{
	Q_ASSERT(m_efx != NULL);

	setWindowTitle(QString("EFX Editor - ") + text);

	m_efx->setName(text);
}

void EFXEditor::slotAlgorithmSelected(const QString &text)
{
	Q_ASSERT(m_efx != NULL);

	m_efx->setAlgorithm(text);

	if (m_efx->isFrequencyEnabled())
	{
		m_xFrequencyLabel->setEnabled(true);
		m_yFrequencyLabel->setEnabled(true);

		m_xFrequencySpin->setEnabled(true);
		m_yFrequencySpin->setEnabled(true);
	}
	else
	{
		m_xFrequencyLabel->setEnabled(false);
		m_yFrequencyLabel->setEnabled(false);

		m_xFrequencySpin->setEnabled(false);
		m_yFrequencySpin->setEnabled(false);
	}

	if (m_efx->isPhaseEnabled())
	{
		m_xPhaseLabel->setEnabled(true);
		m_yPhaseLabel->setEnabled(true);

		m_xPhaseSpin->setEnabled(true);
		m_yPhaseSpin->setEnabled(true);
	}
	else
	{
		m_xPhaseLabel->setEnabled(false);
		m_yPhaseLabel->setEnabled(false);

		m_xPhaseSpin->setEnabled(false);
		m_yPhaseSpin->setEnabled(false);
	}


	m_previewArea->repaint();
}

void EFXEditor::slotWidthSpinChanged(int value)
{
	Q_ASSERT(m_efx != NULL);

	m_efx->setWidth(value);

	m_previewArea->repaint();
}

void EFXEditor::slotHeightSpinChanged(int value)
{
	Q_ASSERT(m_efx != NULL);

	m_efx->setHeight(value);

	m_previewArea->repaint();
}

void EFXEditor::slotRotationSpinChanged(int value)
{
	Q_ASSERT(m_efx != NULL);

	m_efx->setRotation(value);

	m_previewArea->repaint();
}

void EFXEditor::slotXOffsetSpinChanged(int value)
{
	Q_ASSERT(m_efx != NULL);

	m_efx->setXOffset(value);

	m_previewArea->repaint();
}

void EFXEditor::slotYOffsetSpinChanged(int value)
{
	Q_ASSERT(m_efx != NULL);

	m_efx->setYOffset(value);

	m_previewArea->repaint();
}

void EFXEditor::slotXFrequencySpinChanged(int value)
{
	Q_ASSERT(m_efx != NULL);

	m_efx->setXFrequency(value);

	m_previewArea->repaint();
}

void EFXEditor::slotYFrequencySpinChanged(int value)
{
	Q_ASSERT(m_efx != NULL);

	m_efx->setYFrequency(value);

	m_previewArea->repaint();
}

void EFXEditor::slotXPhaseSpinChanged(int value)
{
	Q_ASSERT(m_efx != NULL);

	m_efx->setXPhase(value);

	m_previewArea->repaint();
}

void EFXEditor::slotYPhaseSpinChanged(int value)
{
	Q_ASSERT(m_efx != NULL);

	m_efx->setYPhase(value);

	m_previewArea->repaint();
}

void EFXEditor::slotXAxisActivated(int channel)
{
	Q_ASSERT(m_efx != NULL);

	m_efx->setXChannel(static_cast<t_channel> (channel));

	m_previewArea->repaint();
}

void EFXEditor::slotYAxisActivated(int channel)
{
	Q_ASSERT(m_efx != NULL);

	m_efx->setYChannel(static_cast<t_channel> (channel));

	m_previewArea->repaint();
}

/*****************************************************************************
 * Run order
 *****************************************************************************/

void EFXEditor::slotLoopClicked()
{
	Q_ASSERT(m_efx != NULL);
	m_efx->setRunOrder(Function::Loop);
}

void EFXEditor::slotSingleShotClicked()
{
	Q_ASSERT(m_efx != NULL);
	m_efx->setRunOrder(Function::SingleShot);
}

void EFXEditor::slotPingPongClicked()
{
	Q_ASSERT(m_efx != NULL);
	m_efx->setRunOrder(Function::PingPong);
}

/*****************************************************************************
 * Direction
 *****************************************************************************/

void EFXEditor::slotForwardClicked()
{
	Q_ASSERT(m_efx != NULL);
	m_efx->setDirection(Function::Forward);
}

void EFXEditor::slotBackwardClicked()
{
	Q_ASSERT(m_efx != NULL);
	m_efx->setDirection(Function::Backward);
}

/*****************************************************************************
 * Start scene
 *****************************************************************************/

void EFXEditor::slotStartSceneGroupToggled(bool state)
{
	Q_ASSERT(m_efx != NULL);

	//m_startSceneList->setEnabled(state);
	m_efx->setStartSceneEnabled(state);

	slotStartSceneListSelectionChanged();
}

void EFXEditor::slotStartSceneListSelectionChanged()
{
	Q_ASSERT(m_efx != NULL);

	QTreeWidgetItem* item = m_startSceneList->currentItem();
	if (item != NULL)
		m_efx->setStartScene(item->text(1).toInt());
}

/*****************************************************************************
 * Stop scene
 *****************************************************************************/

void EFXEditor::slotStopSceneGroupToggled(bool state)
{
	Q_ASSERT(m_efx != NULL);

	//m_stopSceneList->setEnabled(state);
	m_efx->setStopSceneEnabled(state);

	slotStopSceneListSelectionChanged();
}

void EFXEditor::slotStopSceneListSelectionChanged()
{
	Q_ASSERT(m_efx != NULL);
   
	QTreeWidgetItem* item = m_stopSceneList->currentItem();
	if (item != NULL)
		m_efx->setStopScene(item->text(1).toInt());
}

/*****************************************************************************
 * EFX Preview Area implementation
 *****************************************************************************/

/**
 * Constructor
 */
EFXPreviewArea::EFXPreviewArea(QWidget* parent) : QFrame (parent)
{
	QPalette p = palette();
	m_points = new QPolygon();

	p.setColor(QPalette::Window, Qt::white);
	setPalette(p);

	setFrameStyle(StyledPanel | Sunken);
}

/**
 * Destructor
 */
EFXPreviewArea::~EFXPreviewArea()
{
	setUpdatesEnabled(false);

	delete m_points;
	m_points = NULL;
}

/**
 * Get the pointer for the point array that is used
 * to draw the preview
 *
 * @return The point array
 */
QPolygon* EFXPreviewArea::points()
{
	return m_points;
}

/**
 * Paint the points in the point array
 */
void EFXPreviewArea::paintEvent(QPaintEvent* e)
{
	QFrame::paintEvent(e);

	QPainter painter(this);
	QPen pen;
	QPoint point;
	unsigned int i;

	// Draw crosshairs
	painter.setPen(Qt::lightGray);
	painter.drawLine(127, 0, 127, 255);
	painter.drawLine(0, 127, 255, 127);

	// Set pen color to black
	pen.setColor(Qt::black);

	// Use the black pen as the painter
	painter.setPen(pen);

	painter.drawPolygon(*m_points);

	// Draw the points from the point array
	for (i = 0; updatesEnabled() && i < m_points->size(); i++)
	{
		point = m_points->point(i);
		painter.drawEllipse(point.x() - 2, point.y() - 2, 4, 4);
	}
}
