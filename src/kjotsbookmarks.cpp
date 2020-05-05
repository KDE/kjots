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

#include <QItemSelectionModel>

#include "kjotsmodel.h"
#include "kjotstreeview.h"

KJotsBookmarks::KJotsBookmarks(KJotsTreeView *treeView) :
    m_treeView(treeView)
{
}

void KJotsBookmarks::openBookmark(const KBookmark &bookmark, Qt::MouseButtons, Qt::KeyboardModifiers)
{
    if (bookmark.url().scheme() != QStringLiteral("akonadi")) {
        return;
    }
    Q_EMIT openLink(bookmark.url());
}

QString KJotsBookmarks::currentIcon() const
{
    const QModelIndexList rows = m_treeView->selectionModel()->selectedRows();
    if (rows.size() != 1) {
        return QString();
    }
    const QModelIndex idx = rows.first();
    auto collection = idx.data(EntityTreeModel::CollectionRole).value<Collection>();
    if (collection.isValid()) {
        return QStringLiteral("x-office-address-book");
    }
    auto item = idx.data(EntityTreeModel::ItemRole).value<Item>();
    if (item.isValid()) {
        return QStringLiteral("x-office-document");
    }
    return QString();
}

QUrl KJotsBookmarks::currentUrl() const
{
    const QModelIndexList rows = m_treeView->selectionModel()->selectedRows();
    if (rows.size() != 1) {
        return QUrl();
    } else {
        return rows.first().data(KJotsModel::EntityUrlRole).toUrl();
    }
}

QString KJotsBookmarks::currentTitle() const
{
    return m_treeView->captionForSelection(QLatin1String(": "));
}

