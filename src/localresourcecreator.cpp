/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

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

#include "localresourcecreator.h"

#include <QDebug>

#include <KLocalizedString>
#include <KRandom>

#include <AkonadiCore/CollectionFetchJob>
#include <AkonadiCore/AgentInstance>
#include <AkonadiCore/AgentManager>
#include <AkonadiCore/CollectionCreateJob>
#include <AkonadiCore/ItemCreateJob>
#include <AkonadiCore/item.h>
#include <AkonadiCore/EntityDisplayAttribute>
#include <Akonadi/Notes/NoteUtils>

#include <KMime/KMimeMessage>

LocalResourceCreator::LocalResourceCreator(QObject *parent)
    : NoteShared::LocalResourceCreator(parent)
{

}

void LocalResourceCreator::finishCreateResource()
{
    Akonadi::CollectionFetchJob *collectionFetchJob = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(), Akonadi::CollectionFetchJob::FirstLevel, this);
    connect(collectionFetchJob, &Akonadi::CollectionFetchJob::result, this, &LocalResourceCreator::rootFetchFinished);
}

void LocalResourceCreator::rootFetchFinished(KJob *job)
{
    if (job->error()) {
        qWarning() << job->errorString();
        deleteLater();
        return;
    }

    Akonadi::CollectionFetchJob *lastCollectionFetchJob = qobject_cast<Akonadi::CollectionFetchJob *>(job);
    if (!lastCollectionFetchJob) {
        deleteLater();
        return;
    }

    Akonadi::Collection::List list = lastCollectionFetchJob->collections();

    if (list.isEmpty()) {
        qWarning() << "Couldn't find new collection in resource";
        deleteLater();
        return;
    }

    for (const Akonadi::Collection &col : qAsConst(list)) {
        Akonadi::AgentInstance instance = Akonadi::AgentManager::self()->instance(col.resource());
        if (instance.type().identifier() == akonadiNotesInstanceName()) {
            Akonadi::CollectionFetchJob *collectionFetchJob = new Akonadi::CollectionFetchJob(col, Akonadi::CollectionFetchJob::FirstLevel, this);
            collectionFetchJob->setProperty("FetchedCollection", col.id());
            connect(collectionFetchJob, &Akonadi::CollectionFetchJob::result, this, &LocalResourceCreator::topLevelFetchFinished);
            return;
        }
    }
    Q_ASSERT(!"Couldn't find new collection");
    deleteLater();
}

void LocalResourceCreator::topLevelFetchFinished(KJob *job)
{
    if (job->error()) {
        qWarning() << job->errorString();
        deleteLater();
        return;
    }

    Akonadi::CollectionFetchJob *lastCollectionFetchJob = qobject_cast<Akonadi::CollectionFetchJob *>(job);
    if (!lastCollectionFetchJob) {
        deleteLater();
        return;
    }

    Akonadi::Collection::List list = lastCollectionFetchJob->collections();

    if (!list.isEmpty()) {
        deleteLater();
        return;
    }

    Akonadi::Collection::Id id = lastCollectionFetchJob->property("FetchedCollection").toLongLong();

    Akonadi::Collection collection;
    collection.setParentCollection(Akonadi::Collection(id));
    QString title = i18nc("The default name for new books.", "New Book");
    collection.setName(KRandom::randomString(10));
    collection.setContentMimeTypes({Akonadi::Collection::mimeType(), Akonadi::NoteUtils::noteMimeType()});

    Akonadi::EntityDisplayAttribute *eda = new Akonadi::EntityDisplayAttribute();
    eda->setIconName(QStringLiteral("x-office-address-book"));
    eda->setDisplayName(title);
    collection.addAttribute(eda);

    Akonadi::CollectionCreateJob *createJob = new Akonadi::CollectionCreateJob(collection, this);
    connect(createJob, &Akonadi::CollectionCreateJob::result, this, &LocalResourceCreator::createFinished);

}

void LocalResourceCreator::createFinished(KJob *job)
{
    if (job->error()) {
        qWarning() << job->errorString();
        deleteLater();
        return;
    }

    Akonadi::CollectionCreateJob *collectionCreateJob = qobject_cast<Akonadi::CollectionCreateJob *>(job);
    if (!collectionCreateJob) {
        deleteLater();
        return;
    }

    Akonadi::Item item;
    item.setParentCollection(collectionCreateJob->collection());
    item.setMimeType(Akonadi::NoteUtils::noteMimeType());

    Akonadi::NoteUtils::NoteMessageWrapper note(KMime::Message::Ptr(new KMime::Message));
    note.setFrom(QStringLiteral("KJots@KDE5"));
    note.setTitle(i18nc("The default name for new pages.", "New Page"));
    note.setCreationDate(QDateTime::currentDateTime());
    note.setLastModifiedDate(QDateTime::currentDateTime());
    // Need a non-empty body part so that the serializer regards this as a valid message.
    note.setText(QStringLiteral(" "));

    item.setPayload(note.message());
    item.attribute<Akonadi::EntityDisplayAttribute>(Akonadi::Item::AddIfMissing)->setIconName(QStringLiteral("text-plain"));

    auto itemJob = new Akonadi::ItemCreateJob(item,  collectionCreateJob->collection(), this);
    connect(itemJob, &Akonadi::ItemCreateJob::result, this, [this, itemJob](KJob*){
                if (itemJob->error()) {
                    qWarning() << "Failed to create a note:" << itemJob->errorString();
                }
                deleteLater();
            });
}

