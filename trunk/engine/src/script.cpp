/*
  Q Light Controller
  script.cpp

  Copyright (C) Heikki Junnila

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
#include <QUrl>

#include "universearray.h"
#include "mastertimer.h"
#include "qlcmacros.h"
#include "script.h"
#include "doc.h"

#define KXMLQLCScriptContents "Contents"

const QString Script::startFunctionCmd = QString("startfunction");
const QString Script::stopFunctionCmd = QString("stopfunction");

const QString Script::waitCmd = QString("wait");
const QString Script::waitKeyCmd = QString("waitkey");

const QString Script::setDMXCmd = QString("setdmx");
const QString Script::setFixtureCmd = QString("setfixture");

/****************************************************************************
 * Initialization
 ****************************************************************************/

Script::Script(Doc* doc) : Function(doc)
    , m_currentCommand(-1)
    , m_waitCount(0)
{
    setName(tr("New Script"));
}

Script::~Script()
{
}

Function::Type Script::type() const
{
    return Function::Script;
}

Function* Script::createCopy(Doc* doc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new Script(doc);
    if (copy->copyFrom(this) == false || doc->addFunction(copy) == false)
    {
        delete copy;
        copy = NULL;
    }

    return copy;
}

bool Script::copyFrom(const Function* function)
{
    const Script* script = qobject_cast<const Script*> (function);
    if (script == NULL)
        return false;

    setData(script->data());

    return Function::copyFrom(function);
}

/****************************************************************************
 * Script data
 ****************************************************************************/

bool Script::setData(const QString& str)
{
    m_data = str;
    m_lines.clear();

    if (m_data.isEmpty() == false)
    {
        QStringList lines = m_data.split(QRegExp("(\r\n|\n\r|\r|\n)"), QString::KeepEmptyParts);
        foreach (QString line, lines)
            m_lines << tokenizeLine(line + QString("\n"));
    }

    return true;
}

QString Script::data() const
{
    return m_data;
}

QStringList Script::tokenizeLine(const QString& str, bool* ok)
{
    QStringList tokens;
    int left = 0;
    int right = 0;
    QString keyword;
    QString value;

    if (str.simplified().startsWith("//") == true || str.simplified().isEmpty() == true)
    {
        tokens = QStringList(); // Return an empty string list for commented lines
    }
    else
    {
        // Truncate everything after the first comment sign
        QString line = str;
        left = line.indexOf("//");
        if (left != -1)
            line.truncate(left);

        left = 0;
        while (left < line.length())
        {
            // Find the next colon to get the keyword
            right = line.indexOf(":", left);
            if (right == -1)
            {
                qDebug() << "Syntax error:" << line.mid(left);
                if (ok != NULL)
                    *ok = false;
                break;
            }
            else
            {
                // Keyword found
                keyword = line.mid(left, right - left);
                left = right + 1;
            }

            // Find the next whitespace to get the value
            right = line.indexOf(QRegExp("\\s"), left);
            if (right == -1)
            {
                qDebug() << "Syntax error:" << line.mid(left);
                if (ok != NULL)
                    *ok = false;
                break;
            }
            else
            {
                // Value found
                value = line.mid(left, right - left);
                left = right + 1;
            }

            tokens << QString(keyword + ":" + value);
        }
    }

    if (ok != NULL)
        *ok = true;

    return tokens;
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool Script::loadXML(const QDomElement* root)
{
    QDomNode node;
    QDomElement tag;

    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root->attribute(KXMLQLCFunctionType) != typeToString(Function::Script))
    {
        qWarning() << Q_FUNC_INFO << root->attribute(KXMLQLCFunctionType)
                   << "is not a script";
        return false;
    }

    /* Load script contents */
    node = root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();

        if (tag.tagName() == KXMLQLCBus)
        {
            /* Bus */
            setBus(tag.text().toUInt());
        }
        else if (tag.tagName() == KXMLQLCFunctionDirection)
        {
            /* Direction */
            setDirection(Function::stringToDirection(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCFunctionRunOrder)
        {
            /* Run Order */
            setRunOrder(Function::stringToRunOrder(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCScriptContents)
        {
            setData(QUrl::fromPercentEncoding(tag.text().toUtf8()));
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown script tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool Script::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement root;
    QDomElement tag;
    QDomText text;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    /* Function tag */
    root = doc->createElement(KXMLQLCFunction);
    wksp_root->appendChild(root);

    root.setAttribute(KXMLQLCFunctionID, id());
    root.setAttribute(KXMLQLCFunctionType, Function::typeToString(type()));
    root.setAttribute(KXMLQLCFunctionName, name());

    /* Speed bus */
    tag = doc->createElement(KXMLQLCBus);
    root.appendChild(tag);
    tag.setAttribute(KXMLQLCBusRole, KXMLQLCBusFade);
    text = doc->createTextNode(QString::number(busID()));
    tag.appendChild(text);

    /* Direction */
    tag = doc->createElement(KXMLQLCFunctionDirection);
    root.appendChild(tag);
    text = doc->createTextNode(Function::directionToString(m_direction));
    tag.appendChild(text);

    /* Run order */
    tag = doc->createElement(KXMLQLCFunctionRunOrder);
    root.appendChild(tag);
    text = doc->createTextNode(Function::runOrderToString(m_runOrder));
    tag.appendChild(text);

    /* Contents */
    tag = doc->createElement(KXMLQLCScriptContents);
    root.appendChild(tag);
    text = doc->createTextNode(QUrl::toPercentEncoding(m_data));
    tag.appendChild(text);

    return true;
}

/****************************************************************************
 * Running
 ****************************************************************************/

void Script::arm()
{
}

void Script::disarm()
{
}

void Script::preRun(MasterTimer* timer)
{
    m_waitCount = 0;
    m_currentCommand = -1;

    Function::preRun(timer);
}

void Script::write(MasterTimer* timer, UniverseArray* universes)
{
    incrementElapsed();

    if (m_waitCount != 0)
    {
        m_waitCount--;
        // TODO: HTP write
    }
    else
    {
        while (m_currentCommand < m_lines.size())
        {
            m_currentCommand++;
            executeCommand(m_currentCommand, timer, universes);
            if (m_waitCount > 0)
                break;
        }
    }

    if (m_currentCommand >= m_lines.size())
        stop();
}

void Script::executeCommand(int index, MasterTimer* timer, UniverseArray* /*universes*/)
{
    QString errorString;
    QStringList tokens;
    QStringList command;

    Doc* doc = qobject_cast<Doc*> (parent());
    Q_ASSERT(doc != NULL);

    if (index < 0 || index >= m_lines.size())
    {
        qWarning() << "Invalid command index:" << index;
        return;
    }

    tokens = m_lines[index];
    if (tokens.isEmpty() == true)
        goto success;

    command = tokens[0].split(":");
    if (command.size() < 2)
    {
        errorString = QString("Line %1 has an invalid command").arg(index);
        goto error;
    }

    if (command[0] == Script::startFunctionCmd)
    {
        bool ok = false;
        quint32 id = command[1].toUInt(&ok);
        if (ok == false)
            goto error;

        Function* function = doc->function(id);
        if (function != NULL)
        {
            if (function->stopped() == true)
                timer->startFunction(function, true);
            else
                qWarning() << "Function" << function->name() << "is already running";
        }
        else
        {
            errorString = QString("No function with ID %1").arg(id);
            goto error;
        }

        goto success;
    }
    else if (command[0] == Script::stopFunctionCmd)
    {
        bool ok = false;
        quint32 id = command[1].toUInt(&ok);
        if (ok == false)
            goto error;

        Function* function = doc->function(id);
        if (function != NULL)
        {
            if (function->stopped() == false)
                function->stop();
            else
                qWarning() << "Function" << function->name() << "is not running";
        }
        else
        {
            errorString = QString("No function with ID %1").arg(id);
            goto error;
        }

        goto success;
    }
    else if (command[0] == Script::waitCmd)
    {
        double time = 0;
        bool ok = false;

        time = command[1].toDouble(&ok);
        if (ok == false)
            goto error;

        m_waitCount = time * MasterTimer::frequency();

        qDebug() << index << "WAIT" << m_waitCount << "ticks";
        goto success;
    }
    else if (command[0] == Script::waitKeyCmd)
    {
        qDebug() << index << "WAIT KEY" << command[1];
        goto success;
    }
    else if (command[0] == Script::setDMXCmd)
    {
        bool ok = false;
        quint32 addr = 0;
        quint32 uni = 0;
        uchar value = 0;

        addr = command[1].toUInt(&ok);
        if (ok == false)
            goto error;

        for (int tok = 1; tok < tokens.size(); tok++)
        {
            QStringList list = tokens[tok].split(":", QString::SkipEmptyParts);
            list[0] = list[0].toLower().trimmed();
            if (list.size() == 2)
            {
                ok = false;
                if (list[0] == "val" || list[0] == "value")
                    value = uchar(list[1].toUInt(&ok));
                else if (list[0] == "uni" || list[0] == "universe")
                    uni = list[1].toUInt(&ok);
                else
                    goto error;

                if (ok == false)
                    goto error;
            }
        }

        qDebug() << index << "SETDMX" << "A:" << addr << "V:" << value << "U:" << uni;
        goto success;
    }
    else if (command[0] == Script::setFixtureCmd)
    {
        bool ok = false;
        quint32 fxi = 0;
        quint32 ch = 0;
        uchar value = 0;

        fxi = command[1].toUInt(&ok);
        if (ok == false)
            goto error;

        for (int tok = 1; tok < tokens.size(); tok++)
        {
            QStringList list = tokens[tok].split(":", QString::SkipEmptyParts);
            list[0] = list[0].toLower().trimmed();
            if (list.size() == 2)
            {
                ok = false;
                if (list[0] == "val" || list[0] == "value")
                    value = uchar(list[1].toUInt(&ok));
                else if (list[0] == "ch" || list[0] == "channel")
                    ch = list[1].toUInt(&ok);
                else
                    goto error;

                if (ok == false)
                    goto error;
            }
        }

        qDebug() << index << "SETFXI" << "FXI:" << fxi << "CH:" << ch << "VAL:" << value;
        goto success;
    }
    else
    {
        errorString = QString("Unknown command:") + command[0];
    }

error:
    qWarning() << index << "error:" << errorString;

success:
    return;
}
