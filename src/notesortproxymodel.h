/*
    This file is part of KJots.

    SPDX-FileCopyrightText: 2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef NOTESORTPROXYMODEL_H
#define NOTESORTPROXYMODEL_H

#include <QSortFilterProxyModel>

class NoteSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit NoteSortProxyModel(QObject *parent = nullptr);
protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};

#endif // NOTESORTPROXYMODEL_H
