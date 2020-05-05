/*
    This file is part of KJots.

    Copyright (c) 2009 Stephen Kelly <steveire@gmail.com>

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

#include "kjotstreeview.h"
#include "kjotsmodel.h"

#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QMenu>
#include <QMimeData>
#include <QColorDialog>

#include <KActionCollection>
#include <KXMLGUIClient>
#include <KLocalizedString>
#include <KMime/Message>

using namespace Akonadi;

KJotsTreeView::KJotsTreeView(KXMLGUIClient *xmlGuiClient, QWidget *parent)
    : EntityTreeView(xmlGuiClient, parent), m_xmlGuiClient(xmlGuiClient)
{

}

void KJotsTreeView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *popup = new QMenu(this);

    QModelIndexList rows = selectionModel()->selectedRows();

    const bool noselection = rows.isEmpty();
    const bool singleselection = rows.size() == 1;
    const bool multiselection = rows.size() > 1;

    popup->addAction(m_xmlGuiClient->actionCollection()->action(QLatin1String("new_book")));
    if (singleselection) {
        popup->addAction(m_xmlGuiClient->actionCollection()->action(QLatin1String("new_page")));
        popup->addAction(m_xmlGuiClient->actionCollection()->action(QLatin1String(KStandardAction::name(KStandardAction::RenameFile))));

        popup->addAction(m_xmlGuiClient->actionCollection()->action(QLatin1String("copy_link_address")));
        popup->addAction(m_xmlGuiClient->actionCollection()->action(QLatin1String("change_color")));

        popup->addAction(m_xmlGuiClient->actionCollection()->action(QLatin1String("sort_children_alpha")));
        popup->addAction(m_xmlGuiClient->actionCollection()->action(QLatin1String("sort_children_by_date")));
    }
    if (!noselection) {
        popup->addAction(m_xmlGuiClient->actionCollection()->action(QLatin1String("save_to")));
    }
    popup->addSeparator();

    popup->addAction(m_xmlGuiClient->actionCollection()->action(QLatin1String("lock")));
    popup->addAction(m_xmlGuiClient->actionCollection()->action(QLatin1String("unlock")));

    if (singleselection) {
        Item item = rows.at(0).data(KJotsModel::ItemRole).value<Item>();
        if (item.isValid()) {
            popup->addAction(m_xmlGuiClient->actionCollection()->action(QLatin1String("del_page")));
        } else {
            popup->addAction(m_xmlGuiClient->actionCollection()->action(QLatin1String("del_folder")));
        }
    }

    if (multiselection) {
        popup->addAction(m_xmlGuiClient->actionCollection()->action(QLatin1String("del_mult")));
    }

    popup->exec(event->globalPos());

    delete popup;
}

void KJotsTreeView::delayedInitialization()
{
    connect(m_xmlGuiClient->actionCollection()->action(QLatin1String("copy_link_address")), &QAction::triggered, this, &KJotsTreeView::copyLinkAddress);
    connect(m_xmlGuiClient->actionCollection()->action(QLatin1String("change_color")), &QAction::triggered, this, &KJotsTreeView::changeColor);
}

QString KJotsTreeView::captionForSelection(const QString &sep) const
{
    QString caption;

    QModelIndexList selection = selectionModel()->selectedRows();

    int selectionSize = selection.size();
    if (selectionSize > 1) {
        caption = i18n("Multiple selections");
    } else if (selectionSize == 1) {
        QModelIndex index = selection.at(0);
        while (index.isValid()) {
            QModelIndex parentBook = index.parent();

            if (parentBook.isValid()) {
                caption = sep + index.data().toString() + caption;
            } else {
                caption = index.data().toString() + caption;
            }
            index = parentBook;
        }
    }
    return caption;
}

void KJotsTreeView::renameEntry()
{
    const QModelIndexList rows = selectionModel()->selectedRows();
    if (rows.size() != 1) {
        return;
    }
    edit(rows.first());
}

void KJotsTreeView::copyLinkAddress()
{
    const QModelIndexList rows = selectionModel()->selectedRows();
    if (rows.size() != 1) {
        return;
    }
    const QModelIndex idx = rows.first();
    QMimeData *mimeData = new QMimeData();
    mimeData->setText(idx.data().toString());
    mimeData->setUrls({idx.data(KJotsModel::EntityUrlRole).toUrl()});
    QApplication::clipboard()->setMimeData(mimeData);
}

void KJotsTreeView::changeColor()
{
    QColor myColor;
    myColor = QColorDialog::getColor();
    if (myColor.isValid()) {
        const QModelIndexList rows = selectionModel()->selectedRows();
        for (const QModelIndex &idx : rows) {
            model()->setData(idx, myColor, Qt::BackgroundRole);
        }
    }
}

