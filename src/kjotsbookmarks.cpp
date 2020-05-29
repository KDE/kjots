/*
    kjotsbookmarks

    Copyright (C) 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
    Copyright (C) 2002, 2003 Aaron J. Seigo <aseigo@kde.org>
    Copyright (C) 2003 Stanislav Kljuhhin <crz@hot.ee>
    Copyright (C) 2005-2006 Jaison Lee <lee.jaison@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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

