/*
  Q Light Controller
  vcspeeddial.cpp

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
#include <QLayout>
#include <QDebug>

#include "vcspeeddialproperties.h"
#include "speedspinbox.h"
#include "vcspeeddial.h"
#include "speeddial.h"
#include "qlcfile.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

VCSpeedDial::VCSpeedDial(QWidget* parent, Doc* doc)
    : VCWidget(parent, doc)
    , m_speedTypes(VCSpeedDial::Duration)
    , m_dial(NULL)
{
    new QVBoxLayout(this);
    layout()->setMargin(0);

    m_dial = new SpeedDial(this);
    layout()->addWidget(m_dial);
    connect(m_dial, SIGNAL(valueChanged(int)), this, SLOT(slotDialValueChanged(int)));
    connect(m_dial, SIGNAL(tapped()), this, SLOT(slotDialTapped()));

    setCaption(tr("Duration"));
    slotModeChanged(doc->mode());
}

VCSpeedDial::~VCSpeedDial()
{
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget* VCSpeedDial::createCopy(VCWidget* parent)
{
    Q_ASSERT(parent != NULL);

    VCSpeedDial* dial = new VCSpeedDial(parent, m_doc);
    if (dial->copyFrom(this) == false)
    {
        delete dial;
        dial = NULL;
    }

    return dial;
}

bool VCSpeedDial::copyFrom(VCWidget* widget)
{
    VCSpeedDial* dial = qobject_cast<VCSpeedDial*> (widget);
    if (dial == NULL)
        return false;

    m_functions = dial->functions();
    m_speedTypes = dial->speedTypes();

    /* Copy common stuff */
    return VCWidget::copyFrom(widget);
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCSpeedDial::editProperties()
{
    VCSpeedDialProperties sdp(this, m_doc);
    sdp.exec();
}

/*****************************************************************************
 * Caption
 *****************************************************************************/

void VCSpeedDial::setCaption(const QString& text)
{
    VCWidget::setCaption(text);

    Q_ASSERT(m_dial != NULL);
    m_dial->setTitle(text);
}

/*****************************************************************************
 * QLC Mode
 *****************************************************************************/

void VCSpeedDial::slotModeChanged(Doc::Mode mode)
{
    if (mode == Doc::Operate)
    {
        m_dial->setEnabled(true);
    }
    else
    {
        m_dial->setEnabled(false);
    }
}

/****************************************************************************
 * Speed type
 ****************************************************************************/

void VCSpeedDial::setSpeedTypes(VCSpeedDial::SpeedTypes types)
{
    m_speedTypes = types;
}

VCSpeedDial::SpeedTypes VCSpeedDial::speedTypes() const
{
    return m_speedTypes;
}

/****************************************************************************
 * Functions
 ****************************************************************************/

void VCSpeedDial::setFunctions(const QSet <quint32> ids)
{
    m_functions = ids;
}

QSet <quint32> VCSpeedDial::functions() const
{
    return m_functions;
}

void VCSpeedDial::slotDialValueChanged(int ms)
{
    foreach (quint32 id, m_functions)
    {
        Function* function = m_doc->function(id);
        if (function != NULL)
        {
            if (m_speedTypes & Duration)
                function->setDuration(ms);
            if (m_speedTypes & FadeIn)
                function->setFadeInSpeed(ms);
            if (m_speedTypes & FadeOut)
                function->setFadeOutSpeed(ms);
        }
    }
}

void VCSpeedDial::slotDialTapped()
{
    foreach (quint32 id, m_functions)
    {
        Function* function = m_doc->function(id);
        if (function != NULL)
        {
            if (m_speedTypes & Duration)
                function->tap();
        }
    }
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCSpeedDial::loadXML(const QDomElement* root)
{
    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCVCSpeedDial)
    {
        qWarning() << Q_FUNC_INFO << "SpeedDial node not found";
        return false;
    }

    setCaption(root->attribute(KXMLQLCVCCaption));
    setSpeedTypes(VCSpeedDial::SpeedTypes(root->attribute(KXMLQLCVCSpeedDialSpeedTypes).toInt()));

    /* Children */
    QDomNode node = root->firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCVCSpeedDialFunction)
        {
            m_functions << tag.text().toUInt();
        }
        else if (tag.tagName() == KXMLQLCWindowState)
        {
            int x = 0, y = 0, w = 0, h = 0;
            bool visible = true;
            loadXMLWindowState(&tag, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
        }
        else if (tag.tagName() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(&tag);
        }
        else if (tag.tagName() == KXMLQLCVCWidgetInput)
        {
            loadXMLInput(&tag);
        }


        node = node.nextSibling();
    }

    return true;
}

bool VCSpeedDial::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(vc_root != NULL);

    QDomElement root = doc->createElement(KXMLQLCVCSpeedDial);
    vc_root->appendChild(root);

    /* Caption */
    root.setAttribute(KXMLQLCVCCaption, caption());

    /* Speed Type */
    root.setAttribute(KXMLQLCVCSpeedDialSpeedTypes, speedTypes());

    /* Window state */
    saveXMLWindowState(doc, &root);

    /* Appearance */
    saveXMLAppearance(doc, &root);

    /* External input */
    saveXMLInput(doc, &root);

    /* Functions */
    foreach (quint32 id, m_functions)
    {
        QDomElement function = doc->createElement(KXMLQLCVCSpeedDialFunction);
        QDomText functionText = doc->createTextNode(QString::number(id));
        function.appendChild(functionText);
        root.appendChild(function);
    }

    return true;
}

void VCSpeedDial::postLoad()
{
    /* Remove such function IDs that don't exist */
    QMutableSetIterator <quint32> it(m_functions);
    while (it.hasNext() == true)
    {
        it.next();
        Function* function = m_doc->function(it.value());
        if (function == NULL)
            it.remove();
    }
}
