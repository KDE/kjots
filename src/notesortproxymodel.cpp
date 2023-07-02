/*
    This file is part of KJots.

    SPDX-FileCopyrightText: 2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "notesortproxymodel.h"

#include <akonadi_version.h>
#include <Akonadi/EntityTreeModel>

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

#include "moc_notesortproxymodel.cpp"
