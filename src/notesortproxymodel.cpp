/*
    This file is part of KJots.

    Copyright (c) 2020 Igor Poboiko <igor.poboiko@gmail.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "notesortproxymodel.h"

#include <AkonadiCore/EntityTreeModel>

#include "noteshared/notepinattribute.h"

using namespace Akonadi;

NoteSortProxyModel::NoteSortProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

bool NoteSortProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{

    const Item leftItem = left.data(EntityTreeModel::ItemRole).value<Item>();
    const Item rightItem = right.data(EntityTreeModel::ItemRole).value<Item>();
    const bool leftPinned = leftItem.hasAttribute<NoteShared::NotePinAttribute>();
    const bool rightPinned = rightItem.hasAttribute<NoteShared::NotePinAttribute>();

    if (!leftPinned && rightPinned) {
        return true;
    }
    if (!rightPinned && leftPinned) {
        return false;
    }

    return QSortFilterProxyModel::lessThan(left, right);
}
