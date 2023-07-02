/*
    This file is part of KJots.

    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>
                  2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kjotsmodel.h"

#include <QAbstractProxyModel>
#include <QTextDocument>
#include <QIcon>

#include <akonadi_version.h>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/NoteUtils>

#include <KMime/Message>
#include <KPIMTextEdit/TextUtils>
#include <KPIMTextEdit/RichTextComposerImages>
#include <KPIMTextEdit/TextHTMLBuilder>
#include <KPIMTextEdit/MarkupDirector>
#include <KPIMTextEdit/PlainTextMarkupBuilder>
#include <KLocalizedString>
#include <KFormat>

#include "noteshared/notelockattribute.h"
#include "noteshared/notepinattribute.h"

Q_DECLARE_METATYPE(QTextDocument *)
Q_DECLARE_METATYPE(KPIMTextEdit::ImageList)

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

    KPIMTextEdit::TextHTMLBuilder builder;
    KPIMTextEdit::MarkupDirector director(&builder);

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

    KPIMTextEdit::PlainTextMarkupBuilder builder;
    KPIMTextEdit::MarkupDirector director(&builder);

    director.processDocument(document);
    const QString result = builder.getResult();

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
        Item item = updateItem(index.data(ItemRole).value<Item>(), value.value<QTextDocument *>());
        return EntityTreeModel::setData(index, QVariant::fromValue(item), ItemRole);
    }

    return EntityTreeModel::setData(index, value, role);
}

QVariant KJotsModel::data(const QModelIndex &index, int role) const
{
    if (GrantleeObjectRole == role) {
        auto obj = new KJotsEntity(index);
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
        auto document = new QTextDocument();
        if (note.textFormat() == Qt::RichText
                || doc.startsWith(u"<!DOCTYPE")
                || doc.startsWith(u"<html>"))
        {
            document->setHtml(doc);
        } else {
            document->setPlainText(doc);
        }
        document->setModified(false);
        // Loading embedded images
        const QVector<NoteUtils::Attachment> attachments = note.attachments();
        for (const auto &attachment : attachments) {
            if (attachment.mimetype() == QStringLiteral("image/png") && !attachment.contentID().isEmpty()) {
                const QImage img = QImage::fromData(attachment.data(), "PNG");
                document->addResource(QTextDocument::ImageResource,
                                      QUrl(QStringLiteral("cid:")+attachment.contentID()), img);
            }
        }

        m_documents.insert(itemId, document);
        return QVariant::fromValue(document);
    }

    if (role == Qt::DecorationRole && index.column() == Title) {
        const Item item = index.data(ItemRole).value<Item>();
        if (item.isValid() && item.hasAttribute<NoteShared::NoteLockAttribute>()) {
            return QIcon::fromTheme(QStringLiteral("object-locked"));
        }
        if (item.isValid() && item.hasAttribute<NoteShared::NotePinAttribute>()) {
            return QIcon::fromTheme(QStringLiteral("pin"));
        }
        const Collection col = index.data(CollectionRole).value<Collection>();
        if (col.isValid() && col.hasAttribute<NoteShared::NoteLockAttribute>()) {
            return QIcon::fromTheme(QStringLiteral("object-locked"));
        }
    }

    return EntityTreeModel::data(index, role);
}

QVariant KJotsModel::entityData(const Akonadi::Item &item, int column, int role) const
{
    if (item.hasPayload<KMime::Message::Ptr>()) {
        auto message = item.payload<KMime::Message::Ptr>();
        NoteUtils::NoteMessageWrapper note(message);
        if (role == Qt::DisplayRole) {
            switch (column) {
            case Title:
                return note.title();
            case ModificationTime:
                return KFormat().formatRelativeDateTime(note.lastModifiedDate(), QLocale::ShortFormat);
            case CreationTime:
                return KFormat().formatRelativeDateTime(note.creationDate(), QLocale::ShortFormat);
            case Size:
                return KFormat().formatByteSize(message->storageSize());
            }
        } else if (role == Qt::EditRole) {
            switch (column) {
            case Title:
                return note.title();
            case ModificationTime:
                return note.lastModifiedDate();
            case CreationTime:
                return note.creationDate();
            case Size:
                return message->size();
            }

        }
    }
    return EntityTreeModel::entityData(item, column, role);
}

QVariant KJotsModel::entityHeaderData(int section, Qt::Orientation orientation, int role, HeaderGroup headerGroup) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return EntityTreeModel:: entityHeaderData(section, orientation, role, headerGroup);
    }
    if (headerGroup == EntityTreeModel::CollectionTreeHeaders) {
        return i18nc("@title:column", "Name");
    } else if (headerGroup == EntityTreeModel::ItemListHeaders) {
        switch (section) {
        case Title:
            return i18nc("@title:column title of a note", "Title");
        case CreationTime:
            return i18nc("@title:column creation date and time of a note", "Created");
        case ModificationTime:
            return i18nc("@title:column last modification date and time of a note", "Modified");
        case Size:
            return i18nc("@title:column size of a note", "Size");
        }
    }
    return EntityTreeModel::entityHeaderData(section, orientation, role, headerGroup);
}

int KJotsModel::entityColumnCount(HeaderGroup headerGroup) const
{
    if (headerGroup == EntityTreeModel::CollectionTreeHeaders) {
        return 1;
    } else if (headerGroup == EntityTreeModel::ItemListHeaders) {
        return 4;
    } else {
        return EntityTreeModel::entityColumnCount(headerGroup);
    }
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

Item KJotsModel::updateItem(const Item &item, QTextDocument *document)
{
    if (!item.hasPayload<KMime::Message::Ptr>()) {
        return {};
    }
    NoteUtils::NoteMessageWrapper note(item.payload<KMime::Message::Ptr>());
    // Saving embedded images
    const auto images = document->property("images").value<KPIMTextEdit::ImageList>();
    QVector<NoteUtils::Attachment> &attachments = note.attachments();
    attachments.clear();
    attachments.reserve(images.count());
    std::transform(images.cbegin(), images.cend(), std::back_inserter(attachments),
                   [](const QSharedPointer<KPIMTextEdit::EmbeddedImage> &img) {
            NoteUtils::Attachment attachment(img->image, QStringLiteral("image/png"));
            attachment.setDataBase64Encoded(true);
            attachment.setContentID(img->contentID);
            return attachment;
        });
    // Setting text
    bool isRichText = KPIMTextEdit::TextUtils::containsFormatting(document);
    if (isRichText) {
        const QByteArray html = KPIMTextEdit::RichTextComposerImages::imageNamesToContentIds( document->toHtml().toUtf8(), images);
        note.setText( QString::fromUtf8(html), Qt::RichText );
    } else {
        note.setText( document->toPlainText(), Qt::PlainText );
    }
    note.setLastModifiedDate(QDateTime::currentDateTime());

    Item newItem = item;
    newItem.setPayload(note.message());
    return newItem;
}

QString KJotsModel::itemPath(const QModelIndex &index, const QString &sep)
{
    QStringList path;
    QModelIndex curIndex = index;
    while (curIndex.isValid()) {
        path.prepend(curIndex.data().toString());
        curIndex = curIndex.parent();
    }
    return path.join(sep);
}

QModelIndex KJotsModel::etmIndex(const QModelIndex &index)
{
    QModelIndex result = index;
    const QAbstractProxyModel *proxy = qobject_cast<const QAbstractProxyModel *>(index.model());
    while (proxy) {
        result = proxy->mapToSource(result);
        proxy = qobject_cast<const QAbstractProxyModel *>(result.model());
    }
    return result;
}

#include "moc_kjotsmodel.cpp"
