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

//Own Header
#include "kjotsbrowser.h"
#include "kjotsmodel.h"

#include <QHelpEvent>
#include <QToolTip>

#include <KLocalizedString>

KJotsBrowser::KJotsBrowser(QAbstractItemModel *model, QWidget *parent)
    : QTextBrowser(parent)
    , m_model(model)
{
    setWordWrapMode(QTextOption::WordWrap);
}

void KJotsBrowser::delayedInitialization()
{
    connect(this, &KJotsBrowser::anchorClicked, this, [this](const QUrl &url){
        if (!url.toString().startsWith(QLatin1Char('#'))) {
            // QTextBrowser tries to automatically handle the url. We only want it for anchor navigation
            // (i.e. "#page12" links). This can be overridden by setting the source to an invalid QUrl
            setSource(QUrl());
            Q_EMIT linkClicked(url);
        }
    });
}

bool KJotsBrowser::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        tooltipEvent(static_cast<QHelpEvent*>(event));
    }
    return QTextBrowser::event(event);
}

void KJotsBrowser::tooltipEvent(QHelpEvent *event)
{
    // This code is somewhat shared with KJotsEdit
    QUrl url(anchorAt(event->pos()));
    QString message;

    if (url.isValid()) {
        QModelIndex idx;
        if (url.scheme() == QStringLiteral("akonadi")) {
            idx = KJotsModel::modelIndexForUrl(m_model, url);
        } else
        // This is #page_XXX internal links
        if (url.scheme().isEmpty() && url.host().isEmpty() && url.path().isEmpty() && url.query().isEmpty()
                   && url.fragment().startsWith(QLatin1String("page_")))
        {
            bool ok;
            Item::Id id = url.fragment().midRef(5).toInt(&ok);
            const QModelIndexList idxs = EntityTreeModel::modelIndexesForItem(m_model, Item(id));
            if (ok && !idxs.isEmpty()) {
                idx = idxs.first();
            }
        } else {
            message = i18nc("@info:tooltip %1 is hyperlink address", "Click to follow the hyperlink: %1", url.toString(QUrl::RemovePassword));
        }
        if (idx.isValid()) {
            if (idx.data(EntityTreeModel::ItemRole).value<Item>().isValid()) {
                message = i18nc("@info:tooltip %1 is page name", "Click to open page: %1", idx.data().toString());
            } else if (idx.data(EntityTreeModel::CollectionRole).value<Collection>().isValid()) {
                message = i18nc("@info:tooltip %1 is book name", "Click to open book: %1", idx.data().toString());
            }
        }
    }

    if (!message.isEmpty()) {
        QToolTip::showText(event->globalPos(), message);
    } else {
        QToolTip::hideText();
    }
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
