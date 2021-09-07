/*
    This file is part of KJots.

    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef NOTECREATORANDSELECTOR_H
#define NOTECREATORANDSELECTOR_H

#include <QItemSelectionModel>
#include <QTimer>

#include <akonadi_version.h>
#if AKONADI_VERSION >= QT_VERSION_CHECK(5, 18, 41)
#include <Akonadi/Collection>
#include <Akonadi/Item>
#else
#include <AkonadiCore/Collection>
#include <AkonadiCore/Item>
#endif

class KJob;

namespace NoteShared
{

/**
 * @brief Creates and selects the newly created note.
 *
 * The note is created in the supplied collection. That collection is
 * selected in the primaryModel. The new note is selected in the
 * secondaryModel. If the secondaryModel is null, the primaryModel is
 * used to select the note too. That is relevant for mixed tree models like
 * KJots uses.
 */
class NoteCreatorAndSelector : public QObject
{
    Q_OBJECT
public:
    explicit NoteCreatorAndSelector(QItemSelectionModel *primaryModel,
                                    QItemSelectionModel *secondaryModel = nullptr,
                                    QObject *parent = nullptr);

    virtual ~NoteCreatorAndSelector();

    void createNote(const Akonadi::Collection &containerCollection);

private:
    void doCreateNote();

private Q_SLOTS:
    void trySelectCollection();
    void noteCreationFinished(KJob *job);
    void trySelectNote();

private:
    QItemSelectionModel *m_primarySelectionModel;
    QItemSelectionModel *m_secondarySelectionModel;

    Akonadi::Collection::Id m_containerCollectionId;
    Akonadi::Item::Id m_newNoteId;
    QTimer *m_giveupTimer;

};

}

#endif
