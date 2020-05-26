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

#include "notecreatorandselector.h"

#include <KLocalizedString>

#include <KMime/Message>

#include <AkonadiCore/EntityDisplayAttribute>
#include <AkonadiCore/EntityTreeModel>
#include <AkonadiCore/ItemCreateJob>
#include <Akonadi/Notes/NoteUtils>

#include "noteshared_debug.h"

using namespace Akonadi;
using namespace NoteShared;

NoteCreatorAndSelector::NoteCreatorAndSelector(QItemSelectionModel *primaryModel, QItemSelectionModel *secondaryModel, QObject *parent)
    : QObject(parent),
      m_primarySelectionModel(primaryModel),
      m_secondarySelectionModel(secondaryModel == nullptr ? primaryModel : secondaryModel),
      m_containerCollectionId(-1),
      m_newNoteId(-1),
      m_giveupTimer(new QTimer(this))
{
    // If the note doesn't exist after 5 seconds, give up waiting for it.
    m_giveupTimer->setInterval(5000);
    connect(m_giveupTimer, &QTimer::timeout, this, &NoteCreatorAndSelector::deleteLater);
}

NoteCreatorAndSelector::~NoteCreatorAndSelector() = default;

void NoteCreatorAndSelector::createNote(const Akonadi::Collection &containerCollection)
{
    m_containerCollectionId = containerCollection.id();

    if (m_primarySelectionModel == m_secondarySelectionModel) {
        doCreateNote();
    } else {
        m_giveupTimer->start();
        connect(m_primarySelectionModel->model(), &QAbstractItemModel::rowsInserted, this, &NoteCreatorAndSelector::trySelectCollection);
        trySelectCollection();
    }
}

void NoteCreatorAndSelector::trySelectCollection()
{
    QModelIndex idx = EntityTreeModel::modelIndexForCollection(m_primarySelectionModel->model(), Collection(m_containerCollectionId));
    if (!idx.isValid()) {
        return;
    }

    m_giveupTimer->stop();
    m_primarySelectionModel->select(QItemSelection(idx, idx), QItemSelectionModel::Select);
    disconnect(m_primarySelectionModel->model(), &QAbstractItemModel::rowsInserted, this, &NoteCreatorAndSelector::trySelectCollection);
    doCreateNote();
}

void NoteCreatorAndSelector::doCreateNote()
{
    Item newItem;
    newItem.setMimeType(Akonadi::NoteUtils::noteMimeType());

    Akonadi::NoteUtils::NoteMessageWrapper note(KMime::Message::Ptr(new KMime::Message));
    note.setFrom(QStringLiteral("KJots@KDE5"));
    note.setTitle(i18nc("The default name for new pages.", "New Page"));
    note.setCreationDate(QDateTime::currentDateTime());
    note.setLastModifiedDate(QDateTime::currentDateTime());
    // Need a non-empty body part so that the serializer regards this as a valid message.
    note.setText(QStringLiteral(" "));

    newItem.setPayload(note.message());
    newItem.attribute<Akonadi::EntityDisplayAttribute>(Akonadi::Item::AddIfMissing)->setIconName(QStringLiteral("text-plain"));

    auto *job = new Akonadi::ItemCreateJob(newItem, Collection(m_containerCollectionId), this);
    connect(job, &Akonadi::ItemCreateJob::result, this, &NoteCreatorAndSelector::noteCreationFinished);

}

void NoteCreatorAndSelector::noteCreationFinished(KJob *job)
{
    if (job->error()) {
        qCWarning(NOTESHARED_LOG) << job->errorString();
        return;
    }
    auto *createJob = qobject_cast<Akonadi::ItemCreateJob *>(job);
    Q_ASSERT(createJob);

    Item newItem = createJob->item();
    m_newNoteId = newItem.id();

    m_giveupTimer->start();
    connect(m_primarySelectionModel->model(), &QAbstractItemModel::rowsInserted, this, &NoteCreatorAndSelector::trySelectNote);
    trySelectNote();
}

void NoteCreatorAndSelector::trySelectNote()
{
    QModelIndexList list = EntityTreeModel::modelIndexesForItem(m_secondarySelectionModel->model(), Item(m_newNoteId));
    if (list.isEmpty()) {
        return;
    }

    const QModelIndex idx = list.first();
    m_secondarySelectionModel->select(QItemSelection(idx, idx), QItemSelectionModel::ClearAndSelect);
}
