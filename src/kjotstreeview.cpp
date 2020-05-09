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
    : EntityTreeView(xmlGuiClient, parent)
    , m_xmlGuiClient(xmlGuiClient)
{

}

void KJotsTreeView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *popup = new QMenu(this);

    const QModelIndexList rows = selectionModel()->selectedRows();
    const bool noselection = rows.isEmpty();
    const bool singleselection = rows.size() == 1;
    const bool itemSelected = singleselection && rows.first().data(EntityTreeModel::ItemRole).value<Item>().isValid();

    if (singleselection) {
        popup->addAction(m_xmlGuiClient->actionCollection()->action(QStringLiteral("akonadi_note_create")));
        popup->addAction(m_xmlGuiClient->actionCollection()->action(QStringLiteral("akonadi_collection_create")));
        popup->addAction(m_xmlGuiClient->actionCollection()->action(QString::fromLatin1(KStandardAction::name(KStandardAction::RenameFile))));
        if (itemSelected) {
            popup->addAction(m_xmlGuiClient->actionCollection()->action(QStringLiteral("akonadi_item_copy")));
        } else {
            popup->addAction(m_xmlGuiClient->actionCollection()->action(QStringLiteral("akonadi_collection_copy")));
        }

        popup->addAction(m_xmlGuiClient->actionCollection()->action(QStringLiteral("akonadi_change_color")));

        popup->addAction(m_xmlGuiClient->actionCollection()->action(QStringLiteral("sort_children_alpha")));
        popup->addAction(m_xmlGuiClient->actionCollection()->action(QStringLiteral("sort_children_by_date")));
    } else if (!noselection) {
        popup->addAction(m_xmlGuiClient->actionCollection()->action(QStringLiteral("akonadi_collection_create")));
        popup->addAction(m_xmlGuiClient->actionCollection()->action(QStringLiteral("akonadi_change_color")));
    }
    if (!noselection) {
        popup->addAction(m_xmlGuiClient->actionCollection()->action(QStringLiteral("save_to")));
    }
    popup->addSeparator();

    popup->addAction(m_xmlGuiClient->actionCollection()->action(QStringLiteral("akonadi_note_lock")));
    popup->addAction(m_xmlGuiClient->actionCollection()->action(QStringLiteral("akonadi_note_unlock")));

    if (singleselection) {
        if (itemSelected) {
            popup->addAction(m_xmlGuiClient->actionCollection()->action(QStringLiteral("akonadi_item_delete")));
        } else {
            popup->addAction(m_xmlGuiClient->actionCollection()->action(QStringLiteral("akonadi_collection_delete")));
        }
    }

    popup->exec(event->globalPos());

    delete popup;
}

void KJotsTreeView::renameEntry()
{
    const QModelIndexList rows = selectionModel()->selectedRows();
    if (rows.size() != 1) {
        return;
    }
    edit(rows.first());
}
