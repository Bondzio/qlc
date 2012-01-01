/*
  Q Light Controller
  rgbtext.cpp

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

#include <QDomDocument>
#include <QDomElement>
#include <QPainter>
#include <QImage>
#include <QDebug>

#include "rgbtext.h"

RGBText::RGBText()
    : RGBAlgorithm()
{
    m_text = "Q L C  ";
}

RGBText::RGBText(const RGBText& t)
    : RGBAlgorithm()
    , m_text(t.text())
    , m_font(t.font())
{
}

RGBText::~RGBText()
{
}

RGBAlgorithm* RGBText::clone() const
{
    RGBText* txt = new RGBText(*this);
    return static_cast<RGBAlgorithm*> (txt);
}

/****************************************************************************
 * RGBText
 ****************************************************************************/

void RGBText::setText(const QString& str)
{
    m_text = str;
}

QString RGBText::text() const
{
    return m_text;
}

void RGBText::setFont(const QFont& font)
{
    m_font = font;
}

QFont RGBText::font() const
{
    return m_font;
}

/****************************************************************************
 * RGBAlgorithm
 ****************************************************************************/

int RGBText::rgbMapStepCount(const QSize& size)
{
    Q_UNUSED(size);
    return m_text.length();
}

RGBMap RGBText::rgbMap(const QSize& size, uint rgb, int step)
{
    RGBMap map(size.height());

    QImage image(size, QImage::Format_RGB32);
    image.fill(0);

    QPainter p(&image);
    p.setRenderHint(QPainter::TextAntialiasing, false);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setFont(m_font);

    p.setPen(QColor(rgb));
    p.drawText(QRect(0, 0, size.width(), size.height()), Qt::AlignCenter, m_text.mid(step, 1));
    p.end();

    for (int y = 0; y < size.height(); y++)
    {
        map[y].resize(size.width());
        for (int x = 0; x < size.width(); x++)
            map[y][x] = image.pixel(x, y);
    }

    return map;
}

QString RGBText::name() const
{
    return QString("Text");
}

QString RGBText::author() const
{
    return QString("Heikki Junnila");
}

int RGBText::apiVersion() const
{
    return 1;
}

RGBAlgorithm::Type RGBText::type() const
{
    return RGBAlgorithm::Text;
}

bool RGBText::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCRGBAlgorithm)
    {
        qWarning() << Q_FUNC_INFO << "RGB Algorithm node not found";
        return false;
    }

    if (root.attribute(KXMLQLCRGBAlgorithmType) != KXMLQLCRGBText)
    {
        qWarning() << Q_FUNC_INFO << "RGB Algorithm is not Text";
        return false;
    }

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCRGBTextContent)
        {
            setText(tag.text());
        }
        else if (tag.tagName() == KXMLQLCRGBTextFont)
        {
            QFont font;
            if (font.fromString(tag.text()) == true)
                setFont(font);
            else
                qWarning() << Q_FUNC_INFO << "Invalid font:" << tag.text();
        }

        node = node.nextSibling();
    }

    return true;
}

bool RGBText::saveXML(QDomDocument* doc, QDomElement* mtx_root) const
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(mtx_root != NULL);

    QDomElement root = doc->createElement(KXMLQLCRGBAlgorithm);
    root.setAttribute(KXMLQLCRGBAlgorithmType, KXMLQLCRGBText);
    mtx_root->appendChild(root);

    QDomElement content = doc->createElement(KXMLQLCRGBTextContent);
    QDomText contentText = doc->createTextNode(m_text);
    content.appendChild(contentText);
    root.appendChild(content);

    QDomElement font = doc->createElement(KXMLQLCRGBTextFont);
    QDomText fontText = doc->createTextNode(m_font.toString());
    font.appendChild(fontText);
    root.appendChild(font);

    return true;
}
