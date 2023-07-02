/*
    This file is part of KJots.

    SPDX-FileCopyrightText: 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
                  2002, 2003 Aaron J. Seigo <aseigo@kde.org>
                  2003 Stanislav Kljuhhin <crz@hot.ee>
                  2005-2006 Jaison Lee <lee.jaison@gmail.com>
                  2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kjotsbookmarks.h"
#include "kjotsmodel.h"

#include <QItemSelectionModel>

using namespace Akonadi;

KJotsBookmarks::KJotsBookmarks(QItemSelectionModel *model, QObject *parent)
    : QObject(parent)
    , m_model(model)
{
}

void KJotsBookmarks::openBookmark(const KBookmark &bm, Qt::MouseButtons /*mb*/, Qt::KeyboardModifiers /*km*/)
{
    if (bm.url().scheme() != QLatin1String("akonadi")) {
        return;
    }
    Q_EMIT openLink(bm.url());
}

QString KJotsBookmarks::currentIcon() const
{
    const QModelIndexList rows = m_model->selectedRows();
    if (rows.size() != 1) {
        return QString();
    }
    const QModelIndex idx = rows.first();
    const auto collection = idx.data(EntityTreeModel::CollectionRole).value<Collection>();
    if (collection.isValid()) {
        return QStringLiteral("x-office-address-book");
    }
    const auto item = idx.data(EntityTreeModel::ItemRole).value<Item>();
    if (item.isValid()) {
        return QStringLiteral("x-office-document");
    }
    return QString();
}

QUrl KJotsBookmarks::currentUrl() const
{
    const QModelIndexList rows = m_model->selectedRows();
    if (rows.size() != 1) {
        return QUrl();
    } else {
        return rows.first().data(KJotsModel::EntityUrlRole).toUrl();
    }
}

QString KJotsBookmarks::currentTitle() const
{
    const QModelIndexList rows = m_model->selectedRows();
    if (rows.size() != 1) {
        return QString();
    } else {
        return KJotsModel::itemPath(rows.first(), QStringLiteral(": "));
    }
}


#include "moc_kjotsbookmarks.cpp"
