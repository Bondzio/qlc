/*
  Q Light Controller
  cuestackmodel.cpp

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

#include <QApplication>
#include <QPalette>
#include <QBrush>
#include <QDebug>
#include <QIcon>

#include "cuestackmodel.h"
#include "cuestack.h"

CueStackModel::CueStackModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_cueStack(NULL)
{
    qDebug() << Q_FUNC_INFO;
    setHeaderData(0, Qt::Horizontal, tr("Number"), Qt::DisplayRole);
    setHeaderData(1, Qt::Horizontal, tr("Cue"), Qt::DisplayRole);
}

CueStackModel::~CueStackModel()
{
}

void CueStackModel::setCueStack(CueStack* cs)
{
    qDebug() << Q_FUNC_INFO << "old:" << qulonglong(m_cueStack) << "new:" << qulonglong(cs);

    if (m_cueStack != NULL)
    {
        beginRemoveRows(QModelIndex(), 0, m_cueStack->cues().size() - 1);
        disconnect(m_cueStack, SIGNAL(added(int)), this, SLOT(slotAdded(int)));
        disconnect(m_cueStack, SIGNAL(removed(int)), this, SLOT(slotRemoved(int)));
        disconnect(m_cueStack, SIGNAL(changed(int)), this, SLOT(slotChanged(int)));
        disconnect(m_cueStack, SIGNAL(currentCueChanged(int)), this, SLOT(slotCurrentCueChanged(int)));
        m_cueStack = NULL;
        endRemoveRows();
    }

    if (cs != NULL)
    {
        beginInsertRows(QModelIndex(), 0, cs->cues().size() - 1);
        m_cueStack = cs;
        connect(m_cueStack, SIGNAL(added(int)), this, SLOT(slotAdded(int)));
        connect(m_cueStack, SIGNAL(removed(int)), this, SLOT(slotRemoved(int)));
        connect(m_cueStack, SIGNAL(changed(int)), this, SLOT(slotChanged(int)));
        connect(m_cueStack, SIGNAL(currentCueChanged(int)), this, SLOT(slotCurrentCueChanged(int)));
        endInsertRows();
    }
}

CueStack* CueStackModel::cueStack() const
{
    return m_cueStack;
}

/****************************************************************************
 * CueStack slots
 ****************************************************************************/

void CueStackModel::slotAdded(int index)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(m_cueStack != NULL);
    beginInsertRows(QModelIndex(), index, index);
    endInsertRows();
}

void CueStackModel::slotRemoved(int index)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(m_cueStack != NULL);
    beginRemoveRows(QModelIndex(), index, index);
    endRemoveRows();
}

void CueStackModel::slotChanged(int index)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(m_cueStack != NULL);
    emit dataChanged(createIndex(index, 0, 0), createIndex(index, 1, 0));
}

void CueStackModel::slotCurrentCueChanged(int index)
{
    emit dataChanged(createIndex(index, 0, 0), createIndex(index, 1, 0));
}

/****************************************************************************
 * QAbstractItemModel
 ****************************************************************************/

QModelIndex CueStackModel::index(int row, int column, const QModelIndex& parent) const
{
    if (m_cueStack == NULL || parent.isValid() == true) // No parents
        return QModelIndex();
    else
        return createIndex(row, column, 0);
}

QModelIndex CueStackModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int CueStackModel::rowCount(const QModelIndex& parent) const
{
    if (m_cueStack == NULL || parent.isValid() == true) // No parents
        return 0;
    else
        return m_cueStack->cues().size();
}

int CueStackModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid() == true) // No parents
        return 0;
    else
        return 2;
}

QVariant CueStackModel::data(const QModelIndex& index, int role) const
{
    if (m_cueStack == NULL)
        return QVariant();

    QVariant var;
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole)
    {
        if (index.column() == 0)
            var = QVariant(index.row() + 1);
        else if (index.column() == 1)
            var = QVariant(m_cueStack->cues()[index.row()].name());
    }
    else if (role == Qt::DecorationRole)
    {
        if (m_cueStack->currentIndex() == index.row() && index.column() == 0)
            var = QVariant(QIcon(":/current.png"));
    }

    return var;
}
