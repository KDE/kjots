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

#include "kjotsmodel.h"

#include <QTextDocument>
#include <QIcon>

#include <AkonadiCore/ChangeRecorder>
#include <AkonadiCore/EntityDisplayAttribute>
#include <Akonadi/Notes/NoteUtils>
#include <KMime/Message>
#include <KPIMTextEdit/TextUtils>

#include <grantlee/markupdirector.h>
#include <grantlee/texthtmlbuilder.h>
#include <grantlee/plaintextmarkupbuilder.h>

#include "noteshared/notelockattribute.h"

Q_DECLARE_METATYPE(QTextDocument *)

using namespace Akonadi;

KJotsEntity::KJotsEntity(const QModelIndex &index, QObject *parent)
    : QObject(parent)
    , m_index(index)
{
}

void KJotsEntity::setIndex(const QModelIndex &index)
{
    m_index = QPersistentModelIndex(index);
}

QString KJotsEntity::title() const
{
    return m_index.data().toString();
}

QString KJotsEntity::content() const
{
    auto *document = m_index.data(KJotsModel::DocumentRole).value<QTextDocument *>();
    if (!document) {
        return QString();
    }

    Grantlee::TextHTMLBuilder builder;
    Grantlee::MarkupDirector director(&builder);

    director.processDocument(document);
    QString result = builder.getResult();

    return result;
}

QString KJotsEntity::plainContent() const
{
    auto *document = m_index.data(KJotsModel::DocumentRole).value<QTextDocument *>();
    if (!document) {
        return QString();
    }

    Grantlee::PlainTextMarkupBuilder builder;
    Grantlee::MarkupDirector director(&builder);

    director.processDocument(document);
    QString result = builder.getResult();

    return result;
}

QString KJotsEntity::url() const
{
    return m_index.data(KJotsModel::EntityUrlRole).toString();
}

qint64 KJotsEntity::entityId() const
{
    Item item = m_index.data(EntityTreeModel::ItemRole).value<Item>();
    if (!item.isValid()) {
        Collection col = m_index.data(EntityTreeModel::CollectionRole).value<Collection>();
        if (!col.isValid()) {
            return -1;
        }
        return col.id();
    }
    return item.id();
}

bool KJotsEntity::isBook() const
{
    Collection col = m_index.data(EntityTreeModel::CollectionRole).value<Collection>();

    if (col.isValid()) {
        return col.contentMimeTypes().contains(Akonadi::NoteUtils::noteMimeType());
    }
    return false;
}

bool KJotsEntity::isPage() const
{
    Item item = m_index.data(EntityTreeModel::ItemRole).value<Item>();
    if (item.isValid()) {
        return item.hasPayload<KMime::Message::Ptr>();
    }
    return false;
}

QVariantList KJotsEntity::entities() const
{
    const QAbstractItemModel *model = m_index.model();
    QVariantList list;
    int row = 0;
    const int column = 0;
    QModelIndex childIndex = model->index(row++, column, m_index);
    while (childIndex.isValid()) {
        auto *obj = new KJotsEntity(childIndex);
        list << QVariant::fromValue(obj);
        childIndex = model->index(row++, column, m_index);
    }
    return list;
}

QVariantList KJotsEntity::breadcrumbs() const
{
    QVariantList list;
    QModelIndex parent = m_index.parent();

    while (parent.isValid()) {
        QObject *obj = new KJotsEntity(parent);
        list << QVariant::fromValue(obj);
        parent = parent.parent();
    }
    return list;
}

KJotsModel::KJotsModel(ChangeRecorder *monitor, QObject *parent)
    : EntityTreeModel(monitor, parent)
{

}

KJotsModel::~KJotsModel()
{
    qDeleteAll(m_documents);
}

bool KJotsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        Item item = index.data(ItemRole).value<Item>();

        if (!item.isValid()) {
            Collection col = index.data(CollectionRole).value<Collection>();
            col.setName(value.toString());
            if (col.hasAttribute<EntityDisplayAttribute>()) {
                auto *eda = col.attribute<EntityDisplayAttribute>();
                eda->setDisplayName(value.toString());
            }
            return EntityTreeModel::setData(index, QVariant::fromValue(col), CollectionRole);
        }
        NoteUtils::NoteMessageWrapper note(item.payload<KMime::Message::Ptr>());
        note.setTitle(value.toString());
        item.setPayload(note.message());

        if (item.hasAttribute<EntityDisplayAttribute>()) {
            auto *displayAttribute = item.attribute<EntityDisplayAttribute>();
            displayAttribute->setDisplayName(value.toString());
        }
        return EntityTreeModel::setData(index, QVariant::fromValue(item), ItemRole);
    }

    if (role == KJotsModel::DocumentRole) {
        Item item = updateItem(index, value.value<QTextDocument *>());
        return EntityTreeModel::setData(index, QVariant::fromValue(item), ItemRole);
    }

    return EntityTreeModel::setData(index, value, role);
}

QVariant KJotsModel::data(const QModelIndex &index, int role) const
{
    if (GrantleeObjectRole == role) {
        auto *obj = new KJotsEntity(index);
        obj->setIndex(index);
        return QVariant::fromValue(obj);
    }

    if (role == KJotsModel::DocumentRole) {
        const Item item = index.data(ItemRole).value<Item>();
        Item::Id itemId = item.id();
        if (m_documents.contains(itemId)) {
            return QVariant::fromValue(m_documents.value(itemId));
        }
        if (!item.hasPayload<KMime::Message::Ptr>()) {
            return QVariant();
        }

        NoteUtils::NoteMessageWrapper note(item.payload<KMime::Message::Ptr>());
        const QString doc = note.text();
        auto *document = new QTextDocument();
        if (note.textFormat() == Qt::RichText
                || doc.startsWith(u"<!DOCTYPE")
                || doc.startsWith(u"<html>"))
        {
            document->setHtml(doc);
        } else {
            document->setPlainText(doc);
        }
        document->setModified(false);
        m_documents.insert(itemId, document);
        return QVariant::fromValue(document);
    }

    if (role == Qt::DecorationRole) {
        const Item item = index.data(ItemRole).value<Item>();
        if (item.isValid() && item.hasAttribute<NoteShared::NoteLockAttribute>()) {
            return QIcon::fromTheme(QStringLiteral("emblem-locked"));
        }
        const Collection col = index.data(CollectionRole).value<Collection>();
        if (col.isValid() && col.hasAttribute<NoteShared::NoteLockAttribute>()) {
            return QIcon::fromTheme(QStringLiteral("emblem-locked"));
        }
    }

    return EntityTreeModel::data(index, role);
}

QVariant KJotsModel::entityData(const Akonadi::Item &item, int column, int role) const
{
    if ((role == Qt::EditRole || role == Qt::DisplayRole) && item.hasPayload<KMime::Message::Ptr>()) {
        NoteUtils::NoteMessageWrapper note(item.payload<KMime::Message::Ptr>());
        return note.title();
    }
    return EntityTreeModel::entityData(item, column, role);
}

QModelIndex KJotsModel::modelIndexForUrl(const QAbstractItemModel *model, const QUrl &url)
{
    if (url.scheme() != QStringLiteral("akonadi")) {
        return {};
    }
    const auto item = Item::fromUrl(url);
    const auto col = Collection::fromUrl(url);
    if (item.isValid()) {
        const QModelIndexList idxs = EntityTreeModel::modelIndexesForItem(model, item);
        if (!idxs.isEmpty()) {
            return idxs.first();
        }
    } else if (col.isValid()) {
        return EntityTreeModel::modelIndexForCollection(model, col);
    }
    return {};
}

Item KJotsModel::updateItem(const QModelIndex &index, QTextDocument *document)
{
    Item item = index.data(ItemRole).value<Item>();
    if (!item.hasPayload<KMime::Message::Ptr>()) {
        return Item();
    }
    NoteUtils::NoteMessageWrapper note(item.payload<KMime::Message::Ptr>());
    bool isRichText = KPIMTextEdit::TextUtils::containsFormatting(document);
    if (isRichText) {
        note.setText( document->toHtml(), Qt::RichText );
    } else {
        note.setText( document->toPlainText(), Qt::PlainText );
    }
    note.setLastModifiedDate(QDateTime::currentDateTime());
    item.setPayload(note.message());
    return item;
}

QString KJotsModel::itemPath(const QModelIndex &index, const QString &sep)
{
    QString caption;
    QModelIndex curIndex = index;
    while (curIndex.isValid()) {
        QModelIndex parentBook = curIndex.parent();
        if (parentBook.isValid()) {
            caption = sep + curIndex.data().toString() + caption;
        } else {
            caption = curIndex.data().toString() + caption;
        }
        curIndex = parentBook;
    }
    return caption;
}
