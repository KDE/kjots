/*
    This file is part of KJots.

    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>
                  2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KJOTSMODEL_H
#define KJOTSMODEL_H

#include <akonadi_version.h>
#include <QtGlobal> // for QT_VERSION_CHECK
#if AKONADI_VERSION >= QT_VERSION_CHECK(5, 18, 41)
#include <Akonadi/EntityTreeModel>
#else
#include <AkonadiCore/EntityTreeModel>
#endif

class QTextDocument;

namespace Akonadi
{
class ChangeRecorder;
}

/**
 * A wrapper QObject making some book and page properties available to Grantlee.
 */
class KJotsEntity : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(QString content READ content)
    Q_PROPERTY(QString plainContent READ plainContent)
    Q_PROPERTY(QString url READ url)
    Q_PROPERTY(qint64 entityId READ entityId)
    Q_PROPERTY(bool isBook READ isBook)
    Q_PROPERTY(bool isPage READ isPage)
    Q_PROPERTY(QVariantList entities READ entities)
    Q_PROPERTY(QVariantList breadcrumbs READ breadcrumbs)

public:
    explicit KJotsEntity(const QModelIndex &index, QObject *parent = nullptr);
    void setIndex(const QModelIndex &index);

    bool isBook() const;
    bool isPage() const;

    QString title() const;

    QString content() const;

    QString plainContent() const;

    QString url() const;

    qint64 entityId() const;

    QVariantList entities() const;

    QVariantList breadcrumbs() const;

private:
    QPersistentModelIndex m_index;
};

class KJotsModel : public Akonadi::EntityTreeModel
{
    Q_OBJECT
public:
    explicit KJotsModel(Akonadi::ChangeRecorder *monitor, QObject *parent = nullptr);
    ~KJotsModel();

    enum KJotsRoles {
        GrantleeObjectRole = EntityTreeModel::UserRole,
        DocumentRole
    };

    enum Column {
        // Title of a note
        Title,

        // Modification date / time of a note
        ModificationTime,

        // Creation date / time of a note
        CreationTime,

        // Size of a note
        Size
    };

    // We don't reimplement the Collection overload.
    using EntityTreeModel::entityData;
    QVariant entityData(const Akonadi::Item &item, int column, int role = Qt::DisplayRole) const override;
    int entityColumnCount(HeaderGroup headerGroup) const override;
    QVariant entityHeaderData(int section, Qt::Orientation orientation, int role, HeaderGroup headerGroup) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    static QModelIndex modelIndexForUrl(const QAbstractItemModel *model, const QUrl &url);
    /**
     * Sets the content of @p item to @p document
     */
    static Akonadi::Item updateItem(const Akonadi::Item &item, QTextDocument *document);
    /**
     * A helper function which returns a full "path" to the @p item (e.g. "Resource / Notebook / Note")
     * using @p sep as a separator. If multiple items are selected, returns "Multiple selection"
     */
    static QString itemPath(const QModelIndex &index, const QString &sep = QStringLiteral(" / "));

    /**
     * A helper function which returns an index belonging to the ETM model through bunch of proxy models
     */
    static QModelIndex etmIndex(const QModelIndex &index);
private:
    QHash<Akonadi::Collection::Id, QColor> m_colors;
    mutable QHash<Akonadi::Item::Id, QTextDocument *> m_documents;
};

#endif

