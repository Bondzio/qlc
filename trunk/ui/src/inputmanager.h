/*
  Q Light Controller
  inputmanager.h

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

#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <QWidget>

class QTreeWidgetItem;
class QTreeWidget;
class QSplitter;
class QTimer;

class InputPatch;
class InputMap;

class InputManager : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(InputManager)

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    /** Get the InputManager singleton instance. Can be NULL. */
    static InputManager* instance();

    /** Create an instance with parent. Fails if s_instance is not NULL. */
    static void createAndShow(QWidget* parent, InputMap* inputMap);

    /** Normal public destructor */
    virtual ~InputManager();

private:
    /** Private constructor to prevent multiple instances. */
    InputManager(QWidget* parent, InputMap* inputMap, Qt::WindowFlags flags = 0);

private:
    static InputManager* s_instance;
    InputMap* m_inputMap;

    /*************************************************************************
     * Tree widget
     *************************************************************************/
public slots:
    /** Update the input mapping tree */
    void updateTree();

private:
    /** Update the contents of an input patch to an item */
    void updateItem(QTreeWidgetItem* item, InputPatch* patch, quint32 universe);

    /** Get the currently opened editor (if any) */
    QWidget* currentEditor() const;

private slots:
    /** Listens to input data and displays a small icon to indicate a
        working connection between a plugin and an input device. */
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value);

    /** Hides the small icon after a while ^^ */
    void slotTimerTimeout();

    /** Displays the InputPatchEditor for currently selected universe */
    void slotEditClicked();

private:
    QSplitter* m_splitter;
    QTreeWidget* m_tree;
    QTimer* m_timer;
};

#endif
