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
#include <QDomDocument>
#include <QDomElement>
#include <QMimeData>
#include <QPalette>
#include <QBrush>
#include <QDebug>
#include <QIcon>

#include "cuestackmodel.h"
#include "cuestack.h"

static int _dragIndex = -1;

CueStackModel::CueStackModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_cueStack(NULL)
{
}

CueStackModel::~CueStackModel()
{
}

void CueStackModel::setCueStack(CueStack* cs)
{
    qDebug() << Q_FUNC_INFO << "old:" << (void*)m_cueStack << "new:" << (void*) cs;

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
    Q_ASSERT(m_cueStack != NULL);
    beginInsertRows(QModelIndex(), index, index);
    endInsertRows();
}

void CueStackModel::slotRemoved(int index)
{
    Q_ASSERT(m_cueStack != NULL);
    beginRemoveRows(QModelIndex(), index, index);
    endRemoveRows();
}

void CueStackModel::slotChanged(int index)
{
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

int CueStackModel::columnCount(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return 2;
}

QVariant CueStackModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    switch(section)
    {
    case 0:
        return tr("Number");
        break;
    case 1:
        return tr("Cue");
        break;
    }
}

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

QStringList CueStackModel::mimeTypes () const
{
    return QStringList() << QString("text/plain");
}

Qt::DropActions CueStackModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}

Qt::ItemFlags CueStackModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (index.isValid() == true)
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

bool CueStackModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row,
                                int column, const QModelIndex& parent)
{
    if (m_cueStack == NULL)
        return false;

    if (data->hasText() == true)
    {
        Cue cue;
        QDomDocument doc;
        if (doc.setContent(data->text()) == true && cue.loadXML(doc.firstChild().firstChild().toElement()) == true)
        {
            int index = parent.row();
            if (_dragIndex < index)
                index++;
            m_cueStack->insertCue(index, cue);
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

QMimeData* CueStackModel::mimeData(const QModelIndexList& indexes) const
{
    if (m_cueStack == NULL)
        return false;

    QDomDocument doc;
    QDomElement root = doc.createElement("MimeData");
    doc.appendChild(root);

    QModelIndex mi = indexes.first();
    if (m_cueStack->cues().at(mi.row()).saveXML(&doc, &root) == true)
    {
        QMimeData* data = new QMimeData;
        data->setText(doc.toString());
        _dragIndex = mi.row();
        return data;
    }
    else
    {
        return NULL;
    }
}

bool CueStackModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (m_cueStack == NULL || parent.isValid() == true)
        return false;

    for (int i = 0; i < count; i++)
        m_cueStack->removeCue(row);

    return true;
}
