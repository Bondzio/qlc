/*
  Q Light Controller
  efxpreviewarea.cpp

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

#include <QPaintEvent>
#include <QPainter>
#include <QDebug>
#include <QPen>

#include "efxpreviewarea.h"
#include "qlcmacros.h"

EFXPreviewArea::EFXPreviewArea(QWidget* parent)
    : QWidget(parent)
    , m_timer(this)
    , m_iter(0)
    , m_reverse(false)
{
    QPalette p = palette();
    p.setColor(QPalette::Window, p.color(QPalette::Base));
    setPalette(p);

    setAutoFillBackground(true);

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
}

EFXPreviewArea::~EFXPreviewArea()
{
}

void EFXPreviewArea::setPoints(const QVector <QPoint>& points)
{
    m_original = QPolygon(points);
    m_points = scale(m_original, size());
}

void EFXPreviewArea::draw(int timerInterval)
{
    m_timer.stop();

    if (m_reverse == true)
        m_iter = m_points.size() - 1;
    else
        m_iter = 0;
    m_timer.start(timerInterval);
}

void EFXPreviewArea::setReverse(bool reverse)
{
    m_reverse = reverse;
}

bool EFXPreviewArea::isReverse() const
{
    return m_reverse;
}

void EFXPreviewArea::slotTimeout()
{
    repaint();
}

QPolygon EFXPreviewArea::scale(const QPolygon& poly, const QSize& target)
{
    QPolygon scaled(poly.size());
    for (int i = 0; i < poly.size(); i++)
    {
        QPoint pt = poly.point(i);
        pt.setX((int) SCALE(qreal(pt.x()), qreal(0), qreal(255), qreal(0), qreal(target.width())));
        pt.setY((int) SCALE(qreal(pt.y()), qreal(0), qreal(255), qreal(0), qreal(target.height())));
        scaled.setPoint(i, pt);
    }

    return scaled;
}

void EFXPreviewArea::resizeEvent(QResizeEvent* e)
{
    m_points = scale(m_original, e->size());
    QWidget::resizeEvent(e);
}

void EFXPreviewArea::paintEvent(QPaintEvent* e)
{
    QWidget::paintEvent(e);

    QPainter painter(this);
    QPen pen;
    QPoint point;
    QColor color;

    /* Crosshairs */
    color = palette().color(QPalette::Mid);
    painter.setPen(color);
    // Do division by two instead with a bitshift to prevent rounding
    painter.drawLine(width() >> 1, 0, width() >> 1, height());
    painter.drawLine(0, height() >> 1, width(), height() >> 1);

    if (m_reverse == true && m_iter >= 0)
        m_iter--;
    else if (m_reverse == false && m_iter < m_points.size())
        m_iter++;

    /* Plain points with text color */
    color = palette().color(QPalette::Text);
    pen.setColor(color);
    painter.setPen(pen);
    painter.drawPolygon(m_points);

    // Draw the points from the point array
    if (m_iter < m_points.size() && m_iter >= 0)
    {
        color = color.lighter(100 + (m_points.size() / 100));
        pen.setColor(color);
        painter.setPen(pen);
        point = m_points.point(m_iter);
        painter.drawEllipse(point.x() - 4, point.y() - 4, 8, 8);
    }
    else
    {
        m_timer.stop();
    }
}
