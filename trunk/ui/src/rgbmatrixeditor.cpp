/*
  Q Light Controller
  rgbmatrixeditor.cpp

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

#include <QGraphicsEllipseItem>
#include <QGraphicsRectItem>
#include <QGraphicsEffect>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QColorDialog>
#include <QFontDialog>
#include <QGradient>
#include <QSettings>
#include <QTimer>
#include <QDebug>

#include "fixtureselection.h"
#include "rgbmatrixeditor.h"
#include "speedspinbox.h"
#include "rgbmatrix.h"
#include "rgbitem.h"
#include "rgbtext.h"
#include "apputil.h"
#include "doc.h"

#define SETTINGS_GEOMETRY "rgbmatrixeditor/geometry"
#define RECT_SIZE 40
#define RECT_PADDING 0
#define ITEM_SIZE 38
#define ITEM_PADDING 2

/****************************************************************************
 * Initialization
 ****************************************************************************/

RGBMatrixEditor::RGBMatrixEditor(QWidget* parent, RGBMatrix* mtx, Doc* doc)
    : QWidget(parent)
    , m_doc(doc)
    , m_mtx(mtx)
    , m_scene(new QGraphicsScene(this))
    , m_previewTimer(new QTimer(this))
    , m_previewIterator(0)
    , m_previewStep(0)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(mtx != NULL);

    setupUi(this);

    // Set a nice gradient backdrop
    m_scene->setBackgroundBrush(Qt::darkGray);
    QLinearGradient gradient(200, 200, 200, 2000);
    gradient.setSpread(QGradient::ReflectSpread);
    m_scene->setBackgroundBrush(gradient);

    connect(m_previewTimer, SIGNAL(timeout()), this, SLOT(slotPreviewTimeout()));
    connect(m_doc, SIGNAL(modeChanged(Doc::Mode)), this, SLOT(slotModeChanged(Doc::Mode)));

    init();
}

RGBMatrixEditor::~RGBMatrixEditor()
{
    m_previewTimer->stop();

    if (m_mtx->stopped() == false)
        m_mtx->stopAndWait();
}

void RGBMatrixEditor::init()
{
    /* Name */
    m_nameEdit->setText(m_mtx->name());
    m_nameEdit->setSelection(0, m_mtx->name().length());
    slotNameEdited(m_mtx->name());

    /* Running order */
    switch (m_mtx->runOrder())
    {
    default:
    case Function::Loop:
        m_loop->setChecked(true);
        break;
    case Function::PingPong:
        m_pingPong->setChecked(true);
        break;
    case Function::SingleShot:
        m_singleShot->setChecked(true);
        break;
    }

    /* Running direction */
    switch (m_mtx->direction())
    {
    default:
    case Function::Forward:
        m_forward->setChecked(true);
        break;
    case Function::Backward:
        m_backward->setChecked(true);
        break;
    }

    /* Speed */
    new QHBoxLayout(m_fadeInContainer);
    m_fadeInSpin = new SpeedSpinBox(SpeedSpinBox::Zero, m_fadeInContainer);
    m_fadeInContainer->layout()->addWidget(m_fadeInSpin);
    m_fadeInContainer->layout()->setMargin(0);
    m_fadeInSpin->setValue(m_mtx->fadeInSpeed());

    new QHBoxLayout(m_fadeOutContainer);
    m_fadeOutSpin = new SpeedSpinBox(SpeedSpinBox::Zero, m_fadeOutContainer);
    m_fadeOutContainer->layout()->addWidget(m_fadeOutSpin);
    m_fadeOutContainer->layout()->setMargin(0);
    m_fadeOutSpin->setValue(m_mtx->fadeOutSpeed());

    new QHBoxLayout(m_durationContainer);
    m_durationSpin = new SpeedSpinBox(SpeedSpinBox::Zero, m_durationContainer);
    m_durationContainer->layout()->addWidget(m_durationSpin);
    m_durationContainer->layout()->setMargin(0);
    m_durationSpin->setValue(m_mtx->duration());

    fillPatternCombo();
    fillFixtureGroupCombo();
    fillAnimationCombo();

    m_fadeInSpin->setValue(m_mtx->fadeInSpeed());
    m_fadeOutSpin->setValue(m_mtx->fadeOutSpeed());
    m_durationSpin->setValue(m_mtx->duration());

    QPixmap pm(100, 26);
    pm.fill(m_mtx->monoColor());
    m_colorButton->setIcon(QIcon(pm));

    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameEdited(const QString&)));
    connect(m_patternCombo, SIGNAL(activated(const QString&)),
            this, SLOT(slotPatternActivated(const QString&)));
    connect(m_fixtureGroupCombo, SIGNAL(activated(int)),
            this, SLOT(slotFixtureGroupActivated(int)));
    connect(m_colorButton, SIGNAL(clicked()),
            this, SLOT(slotColorButtonClicked()));
    connect(m_textEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotTextEdited(const QString&)));
    connect(m_fontButton, SIGNAL(clicked()),
            this, SLOT(slotFontButtonClicked()));
    connect(m_animationCombo, SIGNAL(activated(const QString&)),
            this, SLOT(slotAnimationActivated(const QString&)));
    connect(m_xOffsetSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotOffsetSpinChanged()));
    connect(m_yOffsetSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotOffsetSpinChanged()));

    connect(m_loop, SIGNAL(clicked()), this, SLOT(slotLoopClicked()));
    connect(m_pingPong, SIGNAL(clicked()), this, SLOT(slotPingPongClicked()));
    connect(m_singleShot, SIGNAL(clicked()), this, SLOT(slotSingleShotClicked()));
    connect(m_forward, SIGNAL(clicked()), this, SLOT(slotForwardClicked()));
    connect(m_backward, SIGNAL(clicked()), this, SLOT(slotBackwardClicked()));

    connect(m_fadeInSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotFadeInSpinChanged(int)));
    connect(m_fadeOutSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotFadeOutSpinChanged(int)));
    connect(m_durationSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotDurationSpinChanged(int)));

    // Test slots
    connect(m_testButton, SIGNAL(clicked(bool)),
            this, SLOT(slotTestClicked()));

    createPreviewItems();
    m_preview->setScene(m_scene);
    m_previewTimer->start(MasterTimer::tick());

    updateExtraOptions();
}

void RGBMatrixEditor::fillPatternCombo()
{
    m_patternCombo->addItems(RGBAlgorithm::algorithms());
    if (m_mtx->algorithm() != NULL)
    {
        int index = m_patternCombo->findText(m_mtx->algorithm()->name());
        if (index >= 0)
            m_patternCombo->setCurrentIndex(index);
    }
}

void RGBMatrixEditor::fillFixtureGroupCombo()
{
    m_fixtureGroupCombo->clear();
    m_fixtureGroupCombo->addItem(tr("None"));

    QListIterator <FixtureGroup*> it(m_doc->fixtureGroups());
    while (it.hasNext() == true)
    {
        FixtureGroup* grp(it.next());
        Q_ASSERT(grp != NULL);
        m_fixtureGroupCombo->addItem(grp->name(), grp->id());
        if (m_mtx->fixtureGroup() == grp->id())
            m_fixtureGroupCombo->setCurrentIndex(m_fixtureGroupCombo->count() - 1);
    }
}

void RGBMatrixEditor::fillAnimationCombo()
{
    m_animationCombo->addItems(RGBText::animationStyles());
}

void RGBMatrixEditor::updateExtraOptions()
{
    if (m_mtx->algorithm() == NULL || m_mtx->algorithm()->type() != RGBAlgorithm::Text)
    {
        m_textGroup->hide();
        m_animationGroup->hide();
        m_offsetGroup->hide();
    }
    else
    {
        m_textGroup->show();
        m_animationGroup->show();
        m_offsetGroup->show();

        RGBText* text = static_cast<RGBText*> (m_mtx->algorithm());
        Q_ASSERT(text != NULL);
        m_textEdit->setText(text->text());

        int index = m_animationCombo->findText(RGBText::animationStyleToString(text->animationStyle()));
        if (index != -1)
            m_animationCombo->setCurrentIndex(index);

        m_xOffsetSpin->setValue(text->xOffset());
        m_yOffsetSpin->setValue(text->yOffset());
    }
}

void RGBMatrixEditor::createPreviewItems()
{
    m_previewHash.clear();
    m_scene->clear();

    QPalette pal(this->palette());

    FixtureGroup* grp = m_doc->fixtureGroup(m_mtx->fixtureGroup());
    if (grp == NULL)
    {
        QGraphicsTextItem* text = new QGraphicsTextItem(tr("No fixture group to control"));
        m_scene->addItem(text);
        return;
    }

    m_previewMaps = m_mtx->previewMaps();

    for (int x = 0; x < grp->size().width(); x++)
    {
        for (int y = 0; y < grp->size().height(); y++)
        {
            QLCPoint pt(x, y);

            if (grp->headHash().contains(pt) == true)
            {
                RGBItem* item = new RGBItem;
                item->setRect(x * RECT_SIZE + RECT_PADDING + ITEM_PADDING,
                              y * RECT_SIZE + RECT_PADDING + ITEM_PADDING,
                              ITEM_SIZE - (2 * ITEM_PADDING),
                              ITEM_SIZE - (2 * ITEM_PADDING));
                item->setColor(Qt::black);
                item->draw(0);
                m_scene->addItem(item);
                m_previewHash[pt] = item;

                QGraphicsBlurEffect* blur = new QGraphicsBlurEffect(m_scene);
                blur->setBlurRadius(2);
                item->setGraphicsEffect(blur);
            }
        }
    }

    if (m_mtx->direction() == Function::Forward)
        m_previewStep = 0;
    else
        m_previewStep = m_previewMaps.size() - 1;
}

void RGBMatrixEditor::slotPreviewTimeout()
{
    RGBItem* shape = NULL;

    if (m_mtx->duration() <= 0)
        return;

    m_previewIterator = (m_previewIterator + MasterTimer::tick()) % m_mtx->duration();
    if (m_previewIterator == 0)
    {
        if (m_mtx->direction() == Function::Forward)
        {
            m_previewStep++;
            if (m_previewStep >= m_previewMaps.size())
                m_previewStep = 0;
        }
        else
        {
            m_previewStep--;
            if (m_previewStep < 0)
                m_previewStep = m_previewMaps.size() - 1;
        }
    }

    RGBMap map;
    if (m_previewStep < m_previewMaps.size())
        map = m_previewMaps[m_previewStep];

    for (int y = 0; y < map.size(); y++)
    {
        for (int x = 0; x < map[y].size(); x++)
        {
            QLCPoint pt(x, y);
            if (m_previewHash.contains(pt) == true)
            {
                shape = static_cast<RGBItem*>(m_previewHash[pt]);
                if (shape->color() != QColor(map[y][x]).rgb())
                    shape->setColor(map[y][x]);

                if (shape->color() == QColor(Qt::black).rgb())
                    shape->draw(m_mtx->fadeOutSpeed());
                else
                    shape->draw(m_mtx->fadeInSpeed());
            }
        }
    }
}

void RGBMatrixEditor::slotNameEdited(const QString& text)
{
    setWindowTitle(tr("RGB Matrix - %1").arg(text));
    m_mtx->setName(text);
    m_doc->setModified();
}

void RGBMatrixEditor::slotPatternActivated(const QString& text)
{
    RGBAlgorithm* algo = RGBAlgorithm::algorithm(text);
    m_mtx->setAlgorithm(algo);
    updateExtraOptions();
    slotRestartTest();
}

void RGBMatrixEditor::slotFixtureGroupActivated(int index)
{
    QVariant var = m_fixtureGroupCombo->itemData(index);
    if (var.isValid() == true)
        m_mtx->setFixtureGroup(var.toUInt());
    else
        m_mtx->setFixtureGroup(FixtureGroup::invalidId());
    createPreviewItems();
    slotRestartTest();
}

void RGBMatrixEditor::slotColorButtonClicked()
{
    QColor col = QColorDialog::getColor(m_mtx->monoColor());
    if (col.isValid() == true)
    {
        m_mtx->setMonoColor(col);
        QPixmap pm(100, 26);
        pm.fill(col);
        m_colorButton->setIcon(QIcon(pm));

        slotRestartTest();
    }
}

void RGBMatrixEditor::slotTextEdited(const QString& text)
{
    if (m_mtx->algorithm() != NULL && m_mtx->algorithm()->type() == RGBAlgorithm::Text)
    {
        RGBText* algo = static_cast<RGBText*> (m_mtx->algorithm());
        Q_ASSERT(algo != NULL);
        algo->setText(text);
        slotRestartTest();
    }
}

void RGBMatrixEditor::slotFontButtonClicked()
{
    if (m_mtx->algorithm() != NULL && m_mtx->algorithm()->type() == RGBAlgorithm::Text)
    {
        RGBText* algo = static_cast<RGBText*> (m_mtx->algorithm());
        Q_ASSERT(algo != NULL);

        bool ok = false;
        QFont font = QFontDialog::getFont(&ok, algo->font(), this);
        if (ok == true)
        {
            algo->setFont(font);
            slotRestartTest();
        }
    }
}

void RGBMatrixEditor::slotAnimationActivated(const QString& text)
{
    if (m_mtx->algorithm() != NULL && m_mtx->algorithm()->type() == RGBAlgorithm::Text)
    {
        RGBText* algo = static_cast<RGBText*> (m_mtx->algorithm());
        Q_ASSERT(algo != NULL);
        algo->setAnimationStyle(RGBText::stringToAnimationStyle(text));
        slotRestartTest();
    }
}

void RGBMatrixEditor::slotOffsetSpinChanged()
{
    if (m_mtx->algorithm() != NULL && m_mtx->algorithm()->type() == RGBAlgorithm::Text)
    {
        RGBText* algo = static_cast<RGBText*> (m_mtx->algorithm());
        Q_ASSERT(algo != NULL);
        algo->setXOffset(m_xOffsetSpin->value());
        algo->setYOffset(m_yOffsetSpin->value());
        slotRestartTest();
    }
}

void RGBMatrixEditor::slotLoopClicked()
{
    m_mtx->setRunOrder(Function::Loop);
}

void RGBMatrixEditor::slotPingPongClicked()
{
    m_mtx->setRunOrder(Function::PingPong);
}

void RGBMatrixEditor::slotSingleShotClicked()
{
    m_mtx->setRunOrder(Function::SingleShot);
}

void RGBMatrixEditor::slotForwardClicked()
{
    m_mtx->setDirection(Function::Forward);
}

void RGBMatrixEditor::slotBackwardClicked()
{
    m_mtx->setDirection(Function::Backward);
}

void RGBMatrixEditor::slotFadeInSpinChanged(int ms)
{
    m_mtx->setFadeInSpeed(ms);
}

void RGBMatrixEditor::slotFadeOutSpinChanged(int ms)
{
    m_mtx->setFadeOutSpeed(ms);
}

void RGBMatrixEditor::slotDurationSpinChanged(int ms)
{
    m_mtx->setDuration(ms);
}

void RGBMatrixEditor::slotTestClicked()
{
    if (m_testButton->isChecked() == true)
    {
        m_previewTimer->stop();
        m_mtx->start(m_doc->masterTimer());
    }
    else
    {
        m_mtx->stopAndWait();
        m_previewIterator = 0;
        m_previewTimer->start(MasterTimer::tick());
    }
}

void RGBMatrixEditor::slotRestartTest()
{
    m_previewMaps = m_mtx->previewMaps();

    if (m_testButton->isChecked() == true)
    {
        // Toggle off, toggle on. Duh.
        m_testButton->click();
        m_testButton->click();
    }
}

void RGBMatrixEditor::slotModeChanged(Doc::Mode mode)
{
    if (mode == Doc::Operate)
    {
        if (m_mtx->stopped() == false)
            m_mtx->stopAndWait();
        m_testButton->setChecked(false);
        m_previewTimer->stop();
    }
    else
    {
        m_previewTimer->start();
    }
}
