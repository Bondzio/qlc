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
#include <QGradient>
#include <QSettings>
#include <QTimer>
#include <QDebug>

#include "fixtureselection.h"
#include "rgbmatrixeditor.h"
#include "speedspinbox.h"
#include "rgbmatrix.h"
#include "rgbitem.h"
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
    : QDialog(parent)
    , m_doc(doc)
    , m_original(mtx)
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

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    m_mtx = new RGBMatrix(doc);
    m_mtx->copyFrom(m_original);

    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());
    AppUtil::ensureWidgetIsVisible(this);

    connect(m_previewTimer, SIGNAL(timeout()), this, SLOT(slotPreviewTimeout()));

    init();
}

RGBMatrixEditor::~RGBMatrixEditor()
{
    m_previewTimer->stop();

    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());

    if (m_mtx->stopped() == false)
        m_mtx->stopAndWait();

    delete m_mtx;
    m_mtx = NULL;
}

void RGBMatrixEditor::accept()
{
    m_original->copyFrom(m_mtx);
    QDialog::accept();
}

void RGBMatrixEditor::init()
{
    /* Name */
    m_nameEdit->setText(m_mtx->name());
    m_nameEdit->setSelection(0, m_mtx->name().length());

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

    m_fadeInSpin->setValue(m_mtx->fadeInSpeed());
    m_fadeOutSpin->setValue(m_mtx->fadeOutSpeed());
    m_durationSpin->setValue(m_mtx->duration());

    QPixmap pm(100, 26);
    pm.fill(m_mtx->monoColor());
    m_colorButton->setIcon(QIcon(pm));

    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameEdited(const QString&)));
    connect(m_patternCombo, SIGNAL(activated(int)),
            this, SLOT(slotPatternActivated(int)));
    connect(m_fixtureGroupCombo, SIGNAL(activated(int)),
            this, SLOT(slotFixtureGroupActivated(int)));
    connect(m_colorButton, SIGNAL(clicked()),
            this, SLOT(slotColorButtonClicked()));

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
}

void RGBMatrixEditor::fillPatternCombo()
{
    m_scripts = RGBScript::scripts();
    QListIterator <RGBScript> it(m_scripts);
    while (it.hasNext() == true)
    {
        RGBScript script(it.next());
        if (script.evaluate() == true && script.apiVersion() > 0 &&
            script.name().isEmpty() == false)
        {
            m_patternCombo->addItem(script.name(), script.fileName());
            if (script == m_mtx->script())
                m_patternCombo->setCurrentIndex(m_patternCombo->count() - 1);
        }
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

            if (grp->fixtureHash().contains(pt) == true)
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
    m_mtx->setName(text);
}

void RGBMatrixEditor::slotPatternActivated(int index)
{
    QString fileName(m_patternCombo->itemData(index).toString());
    QListIterator <RGBScript> it(m_scripts);
    while (it.hasNext() == true)
    {
        const RGBScript& script(it.next());
        if (script.fileName() == fileName)
        {
            m_mtx->setScript(script);
            slotRestartTest();
            break;
        }
    }
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
