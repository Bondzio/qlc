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
#include <QMap>
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

    static const QString setHtpCmd;
    static const QString setLtpCmd;
    static const QString setFixtureCmd;

    static const QString labelCmd;
    static const QString jumpCmd;

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
    /** Set the raw script data */
    bool setData(const QString& str);

    /** Get the raw script data */
    QString data() const;

    /** Set, whether to stop started functions when the script itself is stopped. */
    void setStopOwnFunctionsAtEnd(bool stop);

    /** Check, whether to stop started functions when the script itself is stopped. */
    bool stopOwnFunctionsAtEnd() const;

private:
    QString m_data;
    bool m_stopOwnfunctionsAtEnd;

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

    /** @reimpl */
    void postRun(MasterTimer* timer, UniverseArray* universes);

private:
    /**
     * Execute one command from the given line number.
     *
     * @param index Line number to execute
     * @param timer The MasterTimer that runs the house
     * @param universes The universe array that governs DMX data
     */
    void executeCommand(int index, MasterTimer* timer, UniverseArray* universes);

    /**
     * Parse one line of script data into a list of token strings (keyword:value)
     *
     * @param line The script line to parse
     * @param ok Tells if the line was parsed OK or not
     * @return A list of tokens parsed from the line
     */
    static QStringList tokenizeLine(const QString& line, bool* ok = NULL);

private:
    int m_currentCommand;        //! Current command line being handled
    quint32 m_waitCount;         //! Timer ticks to wait before executing the next line
    QList <QStringList> m_lines; //! Raw data parsed into lines of tokens
    QMap <QString,int> m_labels; //! Labels and their line numbers
    QList <Function*> m_startedFunctions; //! Functions started by this script
};

#endif
