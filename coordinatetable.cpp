/*
 * Copyright (c) 2015-2016 Bert De Deckere
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "coordinatetable.h"

CoordinateTable::CoordinateTable(std::vector<Point> &p, QObject *parent) : QAbstractTableModel(parent), p(&p)
{
}

int CoordinateTable::rowCount(const QModelIndex& parent) const
{
    return (*p).size();
}

int CoordinateTable::columnCount(const QModelIndex& parent) const
{
    return 2;
}

QVariant CoordinateTable::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

//    qDebug() << index.row() << " " << index.column() << " " << (*p)[index.row()].x << " " << (*p)[index.row()].y;
    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        if(index.column() == 0)
//            return (*p)[index.row()].x;
            return QString::number((*p)[index.row()].x);
        else
//            return (*p)[index.row()].y;
            return QString::number((*p)[index.row()].y);
    }

    return QVariant();
}

QVariant CoordinateTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0:
                return tr("x [m]");
            case 1:
                return tr("y [m]");
            }
        }
        else
            return QAbstractTableModel::headerData(section, orientation, role);
    }

    return QVariant();
}

bool CoordinateTable::setData(const QModelIndex & index, const QVariant & value, int role)
{
//    qDebug() << "changing entry";
    if(index.isValid())
    {
        if (role == Qt::EditRole)
        {
            if(index.column() == 0)
                (*p)[index.row()].x = value.toDouble();
            else
                (*p)[index.row()].y = value.toDouble();

            return true;
        }
        emit dataChanged(index, index);
    }
    return false;
}

Qt::ItemFlags CoordinateTable::flags(const QModelIndex & /*index*/) const
{
    return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled;
}

void CoordinateTable::updateView()
{
//    QModelIndex topLeft = index(0, 0);
//    QModelIndex bottomRight = index(rowCount() - 1, columnCount() - 1);
//    emit dataChanged(topLeft, bottomRight);
    emit layoutChanged();
}
