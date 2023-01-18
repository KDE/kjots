/*
    This file is part of KJots.

    SPDX-FileCopyrightText: 2009-2010 Tobias Koenig <tokoe@kde.org>
                  2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "standardnoteactionmanager.h"

#include <QAction>
#include <QColorDialog>
#include <QItemSelectionModel>

#include <akonadi_version.h>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/CollectionModifyJob>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/NoteUtils>

#include <KXmlGui/KActionCollection>
#include <KLocalizedString>

#include "notecreatorandselector.h"
#include "notelockattribute.h"
#include "notepinattribute.h"

using namespace Akonadi;

class Q_DECL_HIDDEN StandardNoteActionManager::Private
{
public:
    Private(KActionCollection *actionCollection, QWidget *parentWidget, StandardNoteActionManager *parent)
        : mActionCollection(actionCollection)
        , mParentWidget(parentWidget)
        , mGenericManager(std::make_unique<StandardActionManager>(actionCollection, parentWidget))
        , mParent(parent)
    {
        QObject::connect(mGenericManager.get(), &StandardActionManager::actionStateUpdated,
                         mParent, &StandardNoteActionManager::actionStateUpdated);

        mGenericManager->setMimeTypeFilter({ NoteUtils::noteMimeType() });

        mGenericManager->setCapabilityFilter({ QStringLiteral("Resource") });
    }

    ~Private() = default;

    void updateGenericAllActions()
    {
        updateGenericAction(StandardActionManager::CreateCollection);
        updateGenericAction(StandardActionManager::CopyCollections);
        updateGenericAction(StandardActionManager::DeleteCollections);
        updateGenericAction(StandardActionManager::SynchronizeCollections);
        updateGenericAction(StandardActionManager::CollectionProperties);
        updateGenericAction(StandardActionManager::CopyItems);
        updateGenericAction(StandardActionManager::Paste);
        updateGenericAction(StandardActionManager::DeleteItems);
        updateGenericAction(StandardActionManager::ManageLocalSubscriptions);
        updateGenericAction(StandardActionManager::AddToFavoriteCollections);
        updateGenericAction(StandardActionManager::RemoveFromFavoriteCollections);
        updateGenericAction(StandardActionManager::RenameFavoriteCollection);
        updateGenericAction(StandardActionManager::CopyCollectionToMenu);
        updateGenericAction(StandardActionManager::CopyItemToMenu);
        updateGenericAction(StandardActionManager::MoveItemToMenu);
        updateGenericAction(StandardActionManager::MoveCollectionToMenu);
        updateGenericAction(StandardActionManager::CutItems);
        updateGenericAction(StandardActionManager::CutCollections);
        updateGenericAction(StandardActionManager::CreateResource);
        updateGenericAction(StandardActionManager::DeleteResources);
        updateGenericAction(StandardActionManager::ResourceProperties);
        updateGenericAction(StandardActionManager::SynchronizeResources);
        updateGenericAction(StandardActionManager::ToggleWorkOffline);
        updateGenericAction(StandardActionManager::CopyCollectionToDialog);
        updateGenericAction(StandardActionManager::MoveCollectionToDialog);
        updateGenericAction(StandardActionManager::CopyItemToDialog);
        updateGenericAction(StandardActionManager::MoveItemToDialog);
        updateGenericAction(StandardActionManager::SynchronizeCollectionsRecursive);
        updateGenericAction(StandardActionManager::MoveCollectionsToTrash);
        updateGenericAction(StandardActionManager::MoveItemsToTrash);
        updateGenericAction(StandardActionManager::RestoreCollectionsFromTrash);
        updateGenericAction(StandardActionManager::RestoreItemsFromTrash);
        updateGenericAction(StandardActionManager::MoveToTrashRestoreCollection);
        updateGenericAction(StandardActionManager::MoveToTrashRestoreCollectionAlternative);
        updateGenericAction(StandardActionManager::MoveToTrashRestoreItem);
        updateGenericAction(StandardActionManager::MoveToTrashRestoreItemAlternative);
        updateGenericAction(StandardActionManager::SynchronizeFavoriteCollections);
    }

    void updateGenericAction(StandardActionManager::Type type)
    {
        switch (type) {
        case StandardActionManager::CreateCollection:
            mGenericManager->action(StandardActionManager::CreateCollection)->setText(
                i18n("New Note Book..."));
            mGenericManager->action(StandardActionManager::CreateCollection)->setIcon(
                QIcon::fromTheme(QStringLiteral("address-book-new")));

            mGenericManager->action(StandardActionManager::CreateCollection)->setWhatsThis(
                i18n("Add a new note book to the currently selected bookshelf."));
            mGenericManager->setContextText(
                StandardActionManager::CreateCollection, StandardActionManager::DialogTitle,
                i18nc("@title:window", "New Note Book"));

            mGenericManager->setContextText(
                StandardActionManager::CreateCollection, StandardActionManager::ErrorMessageText,
                ki18n("Could not create note book: %1"));

            mGenericManager->setContextText(
                StandardActionManager::CreateCollection, StandardActionManager::ErrorMessageTitle,
                i18n("Note book creation failed"));
            mGenericManager->action(StandardActionManager::CreateCollection)->setProperty("ContentMimeTypes", { NoteUtils::noteMimeType() });

            break;
        case StandardActionManager::CopyCollections:
            mGenericManager->setActionText(StandardActionManager::CopyCollections,
                                           ki18np("Copy Note Book",
                                                  "Copy %1 Note Books"));
            mGenericManager->action(StandardActionManager::CopyCollections)->setWhatsThis(
                i18n("Copy the selected note books to the clipboard."));
            break;
        case StandardActionManager::DeleteCollections:
            mGenericManager->setActionText(StandardActionManager::DeleteCollections,
                                           ki18np("Delete Note Book",
                                                  "Delete %1 Note Books"));
            mGenericManager->action(StandardActionManager::DeleteCollections)->setWhatsThis(
                i18n("Delete the selected note books from the bookshelf."));

            mGenericManager->setContextText(
                StandardActionManager::DeleteCollections, StandardActionManager::MessageBoxText,
                ki18np("Do you really want to delete this note book and all its contents?",
                       "Do you really want to delete %1 note books and all their contents?"));
            mGenericManager->setContextText(
                StandardActionManager::DeleteCollections, StandardActionManager::MessageBoxTitle,
                ki18ncp("@title:window", "Delete note book?", "Delete note books?"));

            mGenericManager->setContextText(
                StandardActionManager::DeleteCollections, StandardActionManager::ErrorMessageText,
                ki18n("Could not delete note book: %1"));

            mGenericManager->setContextText(
                StandardActionManager::DeleteCollections, StandardActionManager::ErrorMessageTitle,
                i18n("Note book deletion failed"));
            break;
        case StandardActionManager::SynchronizeCollections:
            mGenericManager->setActionText(StandardActionManager::SynchronizeCollections,
                                           ki18np("Update Note Book",
                                                  "Update %1 Note Books"));
            mGenericManager->action(StandardActionManager::SynchronizeCollections)->setWhatsThis(
                i18n("Update the content of the selected note books."));
            break;
        case StandardActionManager::CutCollections:
            mGenericManager->setActionText(StandardActionManager::CutCollections,
                                           ki18np("Cut Note Book",
                                                  "Cut %1 Note Books"));
            mGenericManager->action(StandardActionManager::CutCollections)->setWhatsThis(
                i18n("Cut the selected note books from the bookshelf."));
            break;
        case StandardActionManager::CollectionProperties:
            mGenericManager->action(StandardActionManager::CollectionProperties)->setText(
                i18n("Note Book Properties..."));
            mGenericManager->action(StandardActionManager::CollectionProperties)->setWhatsThis(
                i18n("Open a dialog to edit the properties of the selected note book."));
            mGenericManager->setContextText(
                StandardActionManager::CollectionProperties, StandardActionManager::DialogTitle,
                ki18nc("@title:window", "Properties of Note Book %1"));
            break;

        case StandardActionManager::CopyItems:
            mGenericManager->setActionText(StandardActionManager::CopyItems,
                                           ki18np("Copy Note", "Copy %1 Notes"));
            mGenericManager->action(StandardActionManager::CopyItems)->setWhatsThis(
                i18n("Copy the selected notes to the clipboard."));
            break;
        case StandardActionManager::DeleteItems:
            mGenericManager->setActionText(StandardActionManager::DeleteItems,
                                           ki18np("Delete Note", "Delete %1 Notes"));
            mGenericManager->action(StandardActionManager::DeleteItems)->setWhatsThis(
                i18n("Delete the selected notes from the note book."));
            mGenericManager->setContextText(
                StandardActionManager::DeleteItems, StandardActionManager::MessageBoxText,
                ki18np("Do you really want to delete the selected note?",
                       "Do you really want to delete %1 notes?"));

            mGenericManager->setContextText(
                StandardActionManager::DeleteItems, StandardActionManager::MessageBoxTitle,
                ki18ncp("@title:window", "Delete Note?", "Delete Notes?"));

            mGenericManager->setContextText(
                StandardActionManager::DeleteItems, StandardActionManager::ErrorMessageText,
                ki18n("Could not delete note: %1"));

            mGenericManager->setContextText(
                StandardActionManager::DeleteItems, StandardActionManager::ErrorMessageTitle,
                i18n("Note deletion failed"));
            break;
        case StandardActionManager::CutItems:
            mGenericManager->setActionText(StandardActionManager::CutItems,
                                           ki18np("Cut Note", "Cut %1 Notes"));
            mGenericManager->action(StandardActionManager::CutItems)->setWhatsThis(
                i18n("Cut the selected notes from the note book."));
            break;
        case StandardActionManager::CreateResource:
            mGenericManager->action(StandardActionManager::CreateResource)->setText(
                i18n("Add &Bookshelf..."));
            mGenericManager->action(StandardActionManager::CreateResource)->setWhatsThis(
                i18n("Add a new bookshelf<p>"
                     "You will be presented with a dialog where you can select "
                     "the type of the bookshelf that shall be added.</p>"));
            mGenericManager->setContextText(
                StandardActionManager::CreateResource, StandardActionManager::DialogTitle,
                i18nc("@title:window", "Add Bookshelf"));

            mGenericManager->setContextText(
                StandardActionManager::CreateResource, StandardActionManager::ErrorMessageText,
                ki18n("Could not create bookshelf: %1"));

            mGenericManager->setContextText(
                StandardActionManager::CreateResource, StandardActionManager::ErrorMessageTitle,
                i18n("Bookshelf creation failed"));
            break;
        case StandardActionManager::DeleteResources:

            mGenericManager->setActionText(StandardActionManager::DeleteResources,
                                           ki18np("&Delete Bookshelf",
                                                  "&Delete %1 Bookshelfs"));
            mGenericManager->action(StandardActionManager::DeleteResources)->setWhatsThis(
                i18n("Delete the selected bookshelfs<p>"
                     "The currently selected bookshelfs will be deleted, "
                     "along with all the notes they contain.</p>"));
            mGenericManager->setContextText(
                StandardActionManager::DeleteResources, StandardActionManager::MessageBoxText,
                ki18np("Do you really want to delete this bookshelf?",
                       "Do you really want to delete %1 bookshelfs?"));

            mGenericManager->setContextText(
                StandardActionManager::DeleteResources, StandardActionManager::MessageBoxTitle,
                ki18ncp("@title:window", "Delete Bookshelf?", "Delete Bookshelfs?"));

            break;
        case StandardActionManager::ResourceProperties:

            mGenericManager->action(StandardActionManager::ResourceProperties)->setText(
                i18n("Bookshelf Properties..."));
            mGenericManager->action(StandardActionManager::ResourceProperties)->setWhatsThis(
                i18n("Open a dialog to edit properties of the selected bookshelf."));
            break;
        case StandardActionManager::SynchronizeResources:
            mGenericManager->setActionText(StandardActionManager::SynchronizeResources,
                                           ki18np("Update Bookshelf",
                                                  "Update %1 Bookshelfs"));

            mGenericManager->action(StandardActionManager::SynchronizeResources)->setWhatsThis
                (i18n("Updates the content of all note books of the selected bookshelfs."));

            break;
        case StandardActionManager::Paste:
            mGenericManager->setContextText(
                StandardActionManager::Paste, StandardActionManager::ErrorMessageText,
                ki18n("Could not paste note: %1"));

            mGenericManager->setContextText(
                StandardActionManager::Paste, StandardActionManager::ErrorMessageTitle,
                i18n("Paste failed"));
            break;
        default:
            break;
        }
    }

    void updateActions()
    {
        if (mItemSelectionModel) {
            const Item::List items = mParent->selectedItems();
            const int countUnlocked = std::count_if(items.cbegin(), items.cend(), [](const Item &item){
                    return item.isValid() && !item.hasAttribute<NoteShared::NoteLockAttribute>();
                });

            QAction *action = mActions.value(StandardNoteActionManager::LockUnlockNote);
            if (action) {
                action->setEnabled(items.size() > 0);
                if (countUnlocked > 0) {
                    action->setData(true);
                    action->setText(i18ncp("@action", "Lock Note", "Lock Notes", countUnlocked));
                    action->setIcon(QIcon::fromTheme(QStringLiteral("object-locked")));
                    action->setWhatsThis(i18n("Lock selected notes"));
                } else if (items.size() > 0) {
                    action->setData(false);
                    action->setText(i18ncp("@action", "Unlock Note", "Unlock Notes", items.size()));
                    action->setIcon(QIcon::fromTheme(QStringLiteral("object-unlocked")));
                    action->setWhatsThis(i18n("Unlock selected notes"));
                }
            }

            action = mActions.value(StandardNoteActionManager::PinUnpinNote);
            if (action) {
                const int countUnpinned = std::count_if(items.cbegin(), items.cend(), [](const Item &item){
                        return item.isValid() && !item.hasAttribute<NoteShared::NotePinAttribute>();
                    });

                action->setEnabled(items.size() > 0);
                if (countUnpinned > 0) {
                    action->setData(true);
                    action->setText(i18ncp("@action", "Pin Note", "Pin Notes", countUnpinned));
                    action->setIcon(QIcon::fromTheme(QStringLiteral("window-pin")));
                    action->setWhatsThis(i18n("Pin selected notes"));
                } else if (items.size() > 0) {
                    action->setData(false);
                    action->setText(i18ncp("@action", "Unpin Note", "Unpin Notes", items.size()));
                    action->setIcon(QIcon::fromTheme(QStringLiteral("window-unpin")));
                    action->setWhatsThis(i18n("Unpin selected notes"));
                }
            }

            action = mGenericManager->action(StandardActionManager::DeleteItems);
            if (action) {
                action->setEnabled(countUnlocked == items.size());
            }

            action = mActions.value(StandardNoteActionManager::ChangeNoteColor);
            if (action) {
                action->setEnabled(items.count() > 0);
            }
        } else {
            QAction *action = mActions.value(StandardNoteActionManager::LockUnlockNote);
            if (action) {
                action->setEnabled(false);
            }

            action = mActions.value(StandardNoteActionManager::PinUnpinNote);
            if (action) {
                action->setEnabled(false);
            }

            action = mActions.value(StandardNoteActionManager::ChangeNoteColor);
            if (action) {
                action->setEnabled(false);
            }
        }

        if (mCollectionSelectionModel) {
            const Collection::List collections = mParent->selectedCollections();
            const int countUnlocked = std::count_if(collections.cbegin(), collections.cend(), [](const Collection &collection){
                    return collection.isValid() && !collection.hasAttribute<NoteShared::NoteLockAttribute>();
                });

            QAction *action = mActions.value(StandardNoteActionManager::LockUnlockNoteBook);
            if (action) {
                action->setEnabled(collections.size() > 0);
                if (countUnlocked > 0) {
                    action->setData(true);
                    action->setText(i18ncp("@action", "Lock Note Book", "Lock Note Books", countUnlocked));
                    action->setIcon(QIcon::fromTheme(QStringLiteral("emblem-locked")));
                    action->setWhatsThis(i18n("Lock selected note books"));
                } else if (collections.size() > 0) {
                    action->setData(false);
                    action->setText(i18ncp("@action", "Unlock Note Book", "Unlock Note Books", collections.size()));
                    action->setIcon(QIcon::fromTheme(QStringLiteral("emblem-locked")));
                    action->setWhatsThis(i18n("Unlock selected note books"));
                }
            }

            action = mGenericManager->action(StandardActionManager::DeleteCollections);
            if (action) {
                action->setEnabled(countUnlocked == collections.size());
            }

            action = mActions.value(StandardNoteActionManager::ChangeNoteBookColor);
            if (action) {
                action->setEnabled(collections.count() > 0);
            }
        } else {
            QAction *action = mActions.value(StandardNoteActionManager::LockUnlockNoteBook);
            if (action) {
                action->setEnabled(false);
            }

            action = mActions.value(StandardNoteActionManager::ChangeNoteBookColor);
            if (action) {
                action->setEnabled(false);
            }
        }

        if (mItemSelectionModel && mCollectionSelectionModel) {
            const Collection::List collections = mParent->selectedCollections();
            const Item::List items = mParent->selectedItems();

            QAction *action = mActions.value(StandardNoteActionManager::CreateNote);
            if (action) {
                Akonadi::Collection collection;
                if (collections.count() == 1) {
                    collection = collections.first();
                } else if (collections.count() == 0) {
                    if (items.count() > 0) {
                        collection = mItemSelectionModel->selectedRows().first().data(EntityTreeModel::ParentCollectionRole).value<Collection>();
                    }
                }
                action->setEnabled(collection.isValid() &&
                                   (collection.rights() & Akonadi::Collection::CanCreateItem) &&
                                   (!collection.hasAttribute<NoteShared::NoteLockAttribute>()));
            }
        }

        Q_EMIT mParent->actionStateUpdated();
    }

    void slotLockUnlockNote() {
        if (!mItemSelectionModel || mInterceptedActions.contains(LockUnlockNote)) {
            return;
        }
        const bool lock = mActions[StandardNoteActionManager::LockUnlockNote]->data().toBool();
        const Item::List items = mParent->selectedItems();
        for (auto item : items) {
            if (item.isValid()) {
                if (lock) {
                    item.addAttribute(new NoteShared::NoteLockAttribute);
                } else {
                    item.removeAttribute<NoteShared::NoteLockAttribute>();
                }
                new ItemModifyJob(item, mParent);
            }
        }
    }

    void slotLockUnlockNoteBook() {
        if (!mCollectionSelectionModel || mInterceptedActions.contains(LockUnlockNoteBook)) {
            return;
        }
        const bool lock = mActions[StandardNoteActionManager::LockUnlockNoteBook]->data().toBool();
        const Collection::List collections = mParent->selectedCollections();
        for (auto collection : collections) {
            if (collection.isValid()) {
                if (lock) {
                    collection.addAttribute(new NoteShared::NoteLockAttribute);
                } else {
                    collection.removeAttribute<NoteShared::NoteLockAttribute>();
                }
                new CollectionModifyJob(collection, mParent);
            }
        }
    }

    void slotPinUnpinNote() {
        if (!mItemSelectionModel || mInterceptedActions.contains(PinUnpinNote)) {
            return;
        }
        const bool pin = mActions[StandardNoteActionManager::PinUnpinNote]->data().toBool();
        const Item::List items = mParent->selectedItems();
        for (auto item : items) {
            if (item.isValid()) {
                if (pin) {
                    item.addAttribute(new NoteShared::NotePinAttribute);
                } else {
                    item.removeAttribute<NoteShared::NotePinAttribute>();
                }
                new ItemModifyJob(item, mParent);
            }
        }
    }

    void slotCreateNote() {
        if (mInterceptedActions.contains(CreateNote)) {
            return;
        }
        const Collection::List collections = mParent->selectedCollections();
        if (collections.count() > 1) {
            return;
        }
        Akonadi::Collection collection;
        if (collections.count() == 1) {
            collection = collections.first();
        } else {
            const Item::List items = mParent->selectedItems();
            if (items.isEmpty()) {
                return;
            }
            collection = items.first().parentCollection();
        }
        auto creatorAndSelector = new NoteShared::NoteCreatorAndSelector(mCollectionSelectionModel, mItemSelectionModel, mParent);
        creatorAndSelector->createNote(collection);
    }

    void slotChangeNoteColor() {
        if (mInterceptedActions.contains(ChangeNoteColor)) {
            return;
        }
        QColor color = Qt::white;
        const Item::List items = mParent->selectedItems();
        if (items.size() == 1) {
            const EntityDisplayAttribute *attr = items.first().attribute<EntityDisplayAttribute>();
            if (attr) {
                color = attr->backgroundColor();
            }
        }
        color = QColorDialog::getColor(color, mParentWidget, QString(), QColorDialog::ShowAlphaChannel);
        if (!color.isValid()) {
            return;
        }
        for (auto item : items) {
            item.attribute<EntityDisplayAttribute>(Item::AddIfMissing)->setBackgroundColor(color);
            new ItemModifyJob(item, mParent);
        }
    }

    void slotChangeNoteBookColor() {
        if (mInterceptedActions.contains(ChangeNoteBookColor)) {
            return;
        }
        QColor color = Qt::white;
        const Collection::List collections = mParent->selectedCollections();
        if (collections.size() == 1) {
            const EntityDisplayAttribute *attr = collections.first().attribute<EntityDisplayAttribute>();
            if (attr) {
                color = attr->backgroundColor();
            }
        }
        color = QColorDialog::getColor(color, mParentWidget, QString(), QColorDialog::ShowAlphaChannel);
        if (!color.isValid()) {
            return;
        }
        for (auto collection : collections) {
            collection.attribute<EntityDisplayAttribute>(Collection::AddIfMissing)->setBackgroundColor(color);
            new CollectionModifyJob(collection, mParent);
        }
    }

    KActionCollection *mActionCollection = nullptr;
    QWidget *mParentWidget = nullptr;
    std::unique_ptr<StandardActionManager> mGenericManager;
    QItemSelectionModel *mCollectionSelectionModel = nullptr;
    QItemSelectionModel *mItemSelectionModel = nullptr;
    QHash<StandardNoteActionManager::Type, QAction *> mActions;
    QSet<StandardNoteActionManager::Type> mInterceptedActions;
    StandardNoteActionManager *mParent = nullptr;
};

StandardNoteActionManager::StandardNoteActionManager(KActionCollection *actionCollection, QWidget *parent)
    : QObject(parent)
    , d(std::make_unique<Private>(actionCollection, parent, this))
{
}

StandardNoteActionManager::~StandardNoteActionManager() = default;

void StandardNoteActionManager::setCollectionSelectionModel(QItemSelectionModel *selectionModel)
{
    d->mCollectionSelectionModel = selectionModel;
    d->mGenericManager->setCollectionSelectionModel(selectionModel);

    connect(selectionModel->model(), &QAbstractItemModel::dataChanged, this, [this]() {
        d->updateActions();
    });
    connect(selectionModel->model(), &QAbstractItemModel::rowsInserted, this, [this]() {
        d->updateActions();
    });
    connect(selectionModel->model(), &QAbstractItemModel::rowsRemoved, this, [this]() {
        d->updateActions();
    });
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this, [this]() {
        d->updateActions();
    });

    d->updateActions();
}

void StandardNoteActionManager::setItemSelectionModel(QItemSelectionModel *selectionModel)
{
    d->mItemSelectionModel = selectionModel;
    d->mGenericManager->setItemSelectionModel(selectionModel);

    connect(selectionModel->model(), &QAbstractItemModel::dataChanged, this, [this]() {
        d->updateActions();
    });
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this, [this]() {
        d->updateActions();
    });

    d->updateActions();
}

QAction *StandardNoteActionManager::createAction(Type type)
{
    QAction *action = d->mActions.value(type);
    if (action) {
        return action;
    }

    switch (type) {
    case CreateNote:
        action = new QAction(d->mParentWidget);
        action->setIcon(QIcon::fromTheme(QStringLiteral("document-new")));
        action->setText(i18n("&New Note"));
        action->setWhatsThis(i18n("Add a new note to a selected note book"));
        d->mActions.insert(CreateNote, action);
        d->mActionCollection->addAction(QStringLiteral("akonadi_note_create"), action);
        d->mActionCollection->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::Key_N));
        connect(action, &QAction::triggered, this, [this](){
           d->slotCreateNote();
        });
        break;
    case LockUnlockNote:
        action = new QAction(d->mParentWidget);
        d->mActions.insert(LockUnlockNote, action);
        d->mActionCollection->addAction(QStringLiteral("akonadi_note_lock_unlock"), action);
        connect(action, &QAction::triggered, this, [this](){ d->slotLockUnlockNote(); });
        break;
    case LockUnlockNoteBook:
        action = new QAction(d->mParentWidget);
        d->mActions.insert(LockUnlockNoteBook, action);
        d->mActionCollection->addAction(QStringLiteral("akonadi_notebook_lock_unlock"), action);
        connect(action, &QAction::triggered, this, [this](){ d->slotLockUnlockNoteBook(); });
        break;
    case PinUnpinNote:
        action = new QAction(d->mParentWidget);
        d->mActions.insert(PinUnpinNote, action);
        d->mActionCollection->addAction(QStringLiteral("akonadi_note_pin_unpin"), action);
        connect(action, &QAction::triggered, this, [this](){ d->slotPinUnpinNote(); });
        break;
    case ChangeNoteColor:
        action = new QAction(d->mParentWidget);
        action->setIcon(QIcon::fromTheme(QStringLiteral("format-fill-color")));
        action->setText(i18n("Change Note Color..."));
        action->setWhatsThis(i18n("Changes the color of a selected notes"));
        d->mActions.insert(ChangeNoteColor, action);
        d->mActionCollection->addAction(QStringLiteral("akonadi_note_change_color"), action);
        connect(action, &QAction::triggered, this, [this](){ d->slotChangeNoteColor(); });
        break;
    case ChangeNoteBookColor:
        action = new QAction(d->mParentWidget);
        action->setIcon(QIcon::fromTheme(QStringLiteral("format-fill-color")));
        action->setText(i18n("Change Note Book Color..."));
        action->setWhatsThis(i18n("Changes the color of a selected note books"));
        d->mActions.insert(ChangeNoteColor, action);
        d->mActionCollection->addAction(QStringLiteral("akonadi_notebook_change_color"), action);
        connect(action, &QAction::triggered, this, [this](){ d->slotChangeNoteBookColor(); });
        break;
    default:
        Q_ASSERT(false);   // should never happen
        break;
    }

    return action;
}

QAction *StandardNoteActionManager::createAction(StandardActionManager::Type type)
{
    QAction *act = d->mGenericManager->action(type);
    if (!act) {
        act = d->mGenericManager->createAction(type);
    }
    d->updateGenericAction(type);
    return act;
}

void StandardNoteActionManager::createAllActions()
{
    (void)createAction(CreateNote);
    (void)createAction(LockUnlockNote);
    (void)createAction(LockUnlockNoteBook);
    (void)createAction(PinUnpinNote);
    (void)createAction(ChangeNoteColor);
    (void)createAction(ChangeNoteBookColor);

    d->mGenericManager->createAllActions();
    d->updateGenericAllActions();
    d->updateActions();
}

QAction *StandardNoteActionManager::action(Type type) const
{
    if (d->mActions.contains(type)) {
        return d->mActions.value(type);
    }

    return nullptr;
}

QAction *StandardNoteActionManager::action(StandardActionManager::Type type) const
{
    return d->mGenericManager->action(type);
}

void StandardNoteActionManager::setActionText(StandardActionManager::Type type, const KLocalizedString &text)
{
    d->mGenericManager->setActionText(type, text);
}

void StandardNoteActionManager::interceptAction(Type type, bool intercept)
{
    if (intercept) {
        d->mInterceptedActions.insert(type);
    } else {
        d->mInterceptedActions.remove(type);
    }
}

void StandardNoteActionManager::interceptAction(StandardActionManager::Type type, bool intercept)
{
    d->mGenericManager->interceptAction(type, intercept);
}

Collection::List StandardNoteActionManager::selectedCollections() const
{
    return d->mGenericManager->selectedCollections();
}

Item::List StandardNoteActionManager::selectedItems() const
{
    return d->mGenericManager->selectedItems();
}

void StandardNoteActionManager::setCollectionPropertiesPageNames(const QStringList &names)
{
    d->mGenericManager->setCollectionPropertiesPageNames(names);
}
