/*
  Q Light Controller
  chaserstep.cpp

  Copyright (C) 2004 Heikki Junnila

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
#include <QDomText>
#include <QDebug>

#include "chaserstep.h"
#include "function.h"
#include "doc.h"

ChaserStep::ChaserStep(quint32 aFid, uint aFadeIn, uint aFadeOut, uint aDuration)
    : fid(aFid)
    , fadeIn(aFadeIn)
    , fadeOut(aFadeOut)
    , duration(aDuration)
{
}

ChaserStep::ChaserStep(const ChaserStep& cs)
    : fid(cs.fid)
    , fadeIn(cs.fadeIn)
    , fadeOut(cs.fadeOut)
    , duration(cs.duration)
{
}

bool ChaserStep::operator==(const ChaserStep& cs) const
{
    return (fid == cs.fid) ? true : false;
}

Function* ChaserStep::resolveFunction(const Doc* doc) const
{
    if (doc == NULL)
        return NULL;
    else
        return doc->function(fid);
}

QVariant ChaserStep::toVariant() const
{
    QList <QVariant> list;
    list << fid;
    list << fadeIn;
    list << fadeOut;
    list << duration;
    return list;
}

ChaserStep ChaserStep::fromVariant(const QVariant& var)
{
    ChaserStep cs;
    QList <QVariant> list(var.toList());
    if (list.size() == 4)
    {
        cs.fid = list.takeFirst().toUInt();
        cs.fadeIn = list.takeFirst().toUInt();
        cs.fadeOut = list.takeFirst().toUInt();
        cs.duration = list.takeFirst().toUInt();
    }
    return cs;
}

bool ChaserStep::loadXML(const QDomElement& root, int& stepNumber)
{
    if (root.tagName() != KXMLQLCFunctionStep)
    {
        qWarning() << Q_FUNC_INFO << "ChaserStep node not found";
        return false;
    }

    if (root.hasAttribute(KXMLQLCFunctionSpeedFadeIn) == true)
        fadeIn = root.attribute(KXMLQLCFunctionSpeedFadeIn).toUInt();
    if (root.hasAttribute(KXMLQLCFunctionSpeedFadeOut) == true)
        fadeOut = root.attribute(KXMLQLCFunctionSpeedFadeOut).toUInt();
    if (root.hasAttribute(KXMLQLCFunctionSpeedDuration) == true)
        duration = root.attribute(KXMLQLCFunctionSpeedDuration).toUInt();
    if (root.hasAttribute(KXMLQLCFunctionNumber) == true)
        stepNumber = root.attribute(KXMLQLCFunctionNumber).toInt();

    if (root.text().isEmpty() == false)
        fid = root.text().toUInt();

    return true;
}

bool ChaserStep::saveXML(QDomDocument* doc, QDomElement* root, int stepNumber) const
{
    QDomElement tag;
    QDomText text;

    /* Step tag */
    tag = doc->createElement(KXMLQLCFunctionStep);
    root->appendChild(tag);

    /* Step number */
    tag.setAttribute(KXMLQLCFunctionNumber, stepNumber);

    /* Speeds */
    tag.setAttribute(KXMLQLCFunctionSpeedFadeIn, fadeIn);
    tag.setAttribute(KXMLQLCFunctionSpeedFadeOut, fadeOut);
    tag.setAttribute(KXMLQLCFunctionSpeedDuration, duration);

    /* Step function ID */
    text = doc->createTextNode(QString::number(fid));
    tag.appendChild(text);

    return true;
}
