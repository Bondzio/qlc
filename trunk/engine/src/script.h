/*
  Q Light Controller
  script.h

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

#ifndef SCRIPT_H
#define SCRIPT_H

#include <QStringList>
#include <QObject>
#include "function.h"

class UniverseArray;
class MasterTimer;
class Doc;

class Script : public Function
{
    Q_OBJECT

    /************************************************************************
     * Script keywords
     ************************************************************************/
private:
    static const QString startFunctionCmd;
    static const QString stopFunctionCmd;

    static const QString waitCmd;
    static const QString waitKeyCmd;

    static const QString setDMXCmd;
    static const QString setFixtureCmd;

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    Script(Doc* doc);
    ~Script();

    /** @reimpl */
    Function::Type type() const;

    /** @reimpl */
    Function* createCopy(Doc* doc);

    /** @reimpl */
    bool copyFrom(const Function* function);

    /************************************************************************
     * Script data
     ************************************************************************/
public:
    bool setData(const QString& str);
    QString data() const;

private:
    /**
     * Parse one line of script data into a list of token strings (keyword:value)
     *
     * @param line The script line to parse
     * @param ok Tells if the line was parsed OK or not
     * @return A list of tokens parsed from the line
     */
    QStringList tokenizeLine(const QString& line, bool* ok = NULL);

private:
    QString m_data; //! Raw data
    QList <QStringList> m_lines; //! Raw data parsed into lines of tokens

    /************************************************************************
     * Load & Save
     ************************************************************************/
public:
    /** @reimpl */
    bool loadXML(const QDomElement* root);

    /** @reimpl */
    bool saveXML(QDomDocument* doc, QDomElement* root);

    /************************************************************************
     * Running
     ************************************************************************/
public:
    /** @reimpl */
    void arm();

    /** @reimpl */
    void disarm();

    /** @reimpl */
    void preRun(MasterTimer* timer);

    /** @reimpl */
    void write(MasterTimer* timer, UniverseArray* universes);

    /**
     * Execute one command from the given line number.
     *
     * @param index Line number to execute
     * @param timer The MasterTimer that runs the house
     * @param universes The universe array that governs DMX data
     */
    void executeCommand(int index, MasterTimer* timer, UniverseArray* universes);

private:
    int m_currentCommand;
    quint32 m_waitCount;
};

#endif
