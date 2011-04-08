/*
  Q Light Controller
  qlcclipboard.h

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

#ifndef QLCCLIPBOARD_H
#define QLCCLIPBOARD_H

#include <QList>

namespace QLCClipboardAction {
    enum Action { None, Cut, Copy };
};

template<class T>
class QLCClipboard : private QList <T>
{
    friend class QListIterator <T>;

public:
    inline QLCClipboard() {
        m_action = QLCClipboardAction::None;
    }

    inline ~QLCClipboard() {
    }

    inline QLCClipboardAction::Action action() const {
        return m_action;
    }

    inline void setAction(QLCClipboardAction::Action a) {
        m_action = a;
    }

    inline void cut(const T& t) {
        setAction(QLCClipboardAction::Cut);
        QList<T>::append(t);
    }

    inline void copy(const T& t) {
        setAction(QLCClipboardAction::Copy);
        QList <T>::append(t);
    }

    inline QList <T> paste() {
        return QList <T> (*this);
    }

    inline void removeAll(const T& t) {
        QList<T>::removeAll(t);
    }

    inline void clear() {
        setAction(QLCClipboardAction::None);
        QList<T>::clear();
    }

private:
    QLCClipboardAction::Action m_action;
};

#endif
