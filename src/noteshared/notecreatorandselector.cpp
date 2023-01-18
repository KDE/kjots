/*
    This file is part of KJots.

    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "notecreatorandselector.h"

#include <KLocalizedString>

#include <KMime/Message>

#include <akonadi_version.h>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/NoteUtils>

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
    m_primarySelectionModel->select(idx, QItemSelectionModel::SelectCurrent);
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

    auto job = new Akonadi::ItemCreateJob(newItem, Collection(m_containerCollectionId), this);
    connect(job, &Akonadi::ItemCreateJob::result, this, &NoteCreatorAndSelector::noteCreationFinished);
}

void NoteCreatorAndSelector::noteCreationFinished(KJob *job)
{
    if (job->error()) {
        qCWarning(NOTESHARED_LOG) << job->errorString();
        return;
    }
    auto createJob = qobject_cast<Akonadi::ItemCreateJob *>(job);
    Q_ASSERT(createJob);

    Item newItem = createJob->item();
    m_newNoteId = newItem.id();

    m_giveupTimer->start();
    connect(m_secondarySelectionModel->model(), &QAbstractItemModel::rowsInserted, this, &NoteCreatorAndSelector::trySelectNote);
    trySelectNote();
}

void NoteCreatorAndSelector::trySelectNote()
{
    QModelIndexList list = EntityTreeModel::modelIndexesForItem(m_secondarySelectionModel->model(), Item(m_newNoteId));
    if (list.isEmpty()) {
        return;
    }

    const QModelIndex idx = list.first();
    m_secondarySelectionModel->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

