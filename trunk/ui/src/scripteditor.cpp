/*
  Q Light Controller
  scripteditor.cpp

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

#include <QTextDocument>
#include <QInputDialog>
#include <QTextCursor>
#include <QAction>
#include <QDebug>
#include <QMenu>
#include <cmath>

#include "functionselection.h"
#include "assignhotkey.h"
#include "scripteditor.h"
#include "mastertimer.h"
#include "outputmap.h"
#include "inputmap.h"
#include "script.h"
#include "doc.h"
#include "bus.h"

ScriptEditor::ScriptEditor(QWidget* parent, Script* script, Doc* doc, OutputMap* outputMap,
                           InputMap* inputMap, MasterTimer* masterTimer)
    : QDialog(parent)
    , m_script(script)
    , m_doc(doc)
    , m_outputMap(outputMap)
    , m_inputMap(inputMap)
    , m_masterTimer(masterTimer)
{
    setupUi(this);
    initAddMenu();

    /* Name */
    m_nameEdit->setText(m_script->name());

    /* Document */
    m_document = new QTextDocument(m_script->data(), this);
    m_editor->setDocument(m_document);
}

ScriptEditor::~ScriptEditor()
{
    delete m_document;
    m_document = NULL;
}

void ScriptEditor::accept()
{
    m_script->setName(m_nameEdit->text());
    m_script->setData(m_document->toPlainText());
    QDialog::accept();
}

void ScriptEditor::initAddMenu()
{
    m_addStartFunctionAction = new QAction(QIcon(":/function.png"), tr("Start Function"), this);
    connect(m_addStartFunctionAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddStartFunction()));

    m_addStopFunctionAction = new QAction(QIcon(":/fileclose.png"), tr("Stop Function"), this);
    connect(m_addStopFunctionAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddStopFunction()));

    m_addWaitAction = new QAction(QIcon(":/clock.png"), tr("Wait"), this);
    connect(m_addWaitAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddWait()));

    m_addWaitKeyAction = new QAction(QIcon(":/key_bindings.png"), tr("Wait Key"), this);
    connect(m_addWaitKeyAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddWaitKey()));

    m_addSetDmxAction = new QAction(QIcon(":/fixture.png"), tr("Set DMX"), this);
    connect(m_addSetDmxAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSetDmx()));

    m_addSetFixtureAction = new QAction(QIcon(":/movinghead.png"), tr("Set Fixture"), this);
    connect(m_addSetFixtureAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSetFixture()));

    m_addCommentAction = new QAction(QIcon(":/label.png"), tr("Comment"), this);
    connect(m_addCommentAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddComment()));

    m_addMenu = new QMenu(this);
    m_addMenu->addAction(m_addStartFunctionAction);
    m_addMenu->addAction(m_addStopFunctionAction);
    m_addMenu->addSeparator();
    m_addMenu->addAction(m_addWaitAction);
    m_addMenu->addAction(m_addWaitKeyAction);
    m_addMenu->addSeparator();
    m_addMenu->addAction(m_addSetDmxAction);
    m_addMenu->addAction(m_addSetFixtureAction);
    m_addMenu->addSeparator();
    m_addMenu->addAction(m_addCommentAction);

    m_addButton->setMenu(m_addMenu);
}

void ScriptEditor::slotAddStartFunction()
{
    FunctionSelection fs(this, m_doc, m_outputMap, m_inputMap, m_masterTimer);
    if (fs.exec() == QDialog::Accepted)
    {
        m_editor->moveCursor(QTextCursor::StartOfLine);
        QTextCursor cursor(m_editor->textCursor());

        foreach (quint32 id, fs.selection())
        {
            QString cmd = QString("startfunction:%1\n").arg(id);
            cursor.insertText(cmd);
            m_editor->moveCursor(QTextCursor::Down);
        }
    }
}

void ScriptEditor::slotAddStopFunction()
{
    FunctionSelection fs(this, m_doc, m_outputMap, m_inputMap, m_masterTimer);
    if (fs.exec() == QDialog::Accepted)
    {
        m_editor->moveCursor(QTextCursor::StartOfLine);
        QTextCursor cursor(m_editor->textCursor());

        foreach (quint32 id, fs.selection())
        {
            QString cmd = QString("stopfunction:%1\n").arg(id);
            cursor.insertText(cmd);
            m_editor->moveCursor(QTextCursor::Down);
        }
    }
}

void ScriptEditor::slotAddWait()
{
    bool ok = false;
    double val = QInputDialog::getDouble(this, tr("Wait"), tr("Seconds to wait"),
                                         1.0,     // Default
                                         0,       // Min
                                         INT_MAX, // Max
                                         3,       // Decimals
                                         &ok);
    if (ok == true)
    {
        m_editor->moveCursor(QTextCursor::StartOfLine);
        m_editor->textCursor().insertText(QString("wait:%1\n").arg(val));
    }
}

void ScriptEditor::slotAddWaitKey()
{
    AssignHotKey ahk(this);
    if (ahk.exec() == QDialog::Accepted)
    {
        m_editor->moveCursor(QTextCursor::StartOfLine);
        m_editor->textCursor().insertText(QString("waitkey:%1\n").arg(ahk.keySequence().toString()));
    }
}

void ScriptEditor::slotAddSetDmx()
{
    m_editor->moveCursor(QTextCursor::StartOfLine);
    m_editor->textCursor().insertText(QString("setdmx:0 uni:1 val:0\n"));
}

void ScriptEditor::slotAddSetFixture()
{
}

void ScriptEditor::slotAddComment()
{
    bool ok = false;
    QString str = QInputDialog::getText(this, tr("Add Comment"), "",
                                        QLineEdit::Normal, QString(), &ok);
    if (ok == true)
        m_editor->textCursor().insertText(QString("// %1\n").arg(str));
}
