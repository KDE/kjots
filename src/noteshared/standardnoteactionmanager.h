/*
    This file is part of KJots

    SPDX-FileCopyrightText: 2020 Igor Poboiko <igor.poboiko@gmail.com>
                  2009 - 2010 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef AKONADI_STANDARDNOTESACTIONMANAGER_H
#define AKONADI_STANDARDNOTESACTIONMANAGER_H

#include <AkonadiWidgets/StandardActionManager>

#include <QObject>

class QAction;
class KActionCollection;
class QItemSelectionModel;
class QWidget;

namespace Akonadi {
class Item;

/**
 * @short Manages note specific actions for collection and item views.
 *
 * @author Igor Poboiko <igor.poboiko@gmail.com>
 */
class StandardNoteActionManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Describes the supported actions.
     */
    enum Type {
        CreateNote = StandardActionManager::LastType + 1, ///< Creates a new note
        LockUnlockNote,                                   ///< Locks or unlocks a note
        LockUnlockNoteBook,                               ///< Locks or unlocks a note book
        PinUnpinNote,                                     ///< Pins or unpins a note
        ChangeNoteColor,                                  ///< Changes a color of a note
        ChangeNoteBookColor                               ///< Changes a color of a note book
    };

    /**
     * Creates a new standard note action manager.
     *
     * @param actionCollection The action collection to operate on.
     * @param parent The parent widget.
     */
    explicit StandardNoteActionManager(KActionCollection *actionCollection, QWidget *parent = nullptr);

    /**
     * Destroys the standard note action manager.
     */
    ~StandardNoteActionManager();

    /**
     * Sets the collection selection model based on which the collection
     * related actions should operate. If none is set, all collection actions
     * will be disabled.
     *
     * @param selectionModel selection model for collections
     */
    void setCollectionSelectionModel(QItemSelectionModel *selectionModel);

    /**
     * Sets the item selection model based on which the item related actions
     * should operate. If none is set, all item actions will be disabled.
     *
     * @param selectionModel the selection model for items
     */
    void setItemSelectionModel(QItemSelectionModel *selectionModel);

    /**
     * Creates the action of the given type and adds it to the action collection
     * specified in the constructor if it does not exist yet. The action is
     * connected to its default implementation provided by this class.
     *
     * @param type the type of action to create
     */
    Q_REQUIRED_RESULT QAction *createAction(Type type);

    /**
     * Creates the action of the given type and adds it to the action collection
     * specified in the constructor if it does not exist yet. The action is
     * connected to its default implementation provided by this class.
     *
     * @param type the type of action to create
     */
    QAction *createAction(StandardActionManager::Type type);

    /**
     * Convenience method to create all standard actions.
     * @see createAction()
     */
    void createAllActions();

    /**
     * Returns the action of the given type, 0 if it has not been created (yet).
     */
    QAction *action(Type type) const;

    /**
     * Returns the action of the given type, 0 if it has not been created (yet).
     *
     * @param type the type of action to return
     */
    QAction *action(StandardActionManager::Type type) const;

    /**
     * Sets the label of the action @p type to @p text, which is used during
     * updating the action state and substituted according to the number of
     * selected objects. This is mainly useful to customize the label of actions
     * that can operate on multiple objects.
     *
     * Example:
     * @code
     * acctMgr->setActionText( Akonadi::StandardActionManager::CopyItems,
     *                         ki18np( "Copy Item", "Copy %1 Items" ) );
     * @endcode
     */
    void setActionText(StandardActionManager::Type type, const KLocalizedString &text);

    /**
     * Sets whether the default implementation for the given action @p type
     * shall be executed when the action is triggered.
     *
     * @param intercept If @c false, the default implementation will be executed,
     *                  if @c true no action is taken.
     */
    void interceptAction(Type type, bool intercept = true);

    /**
     * Sets whether the default implementation for the given action @p type
     * shall be executed when the action is triggered.
     *
     * @param intercept If @c false, the default implementation will be executed,
     *                  if @c true no action is taken.
     */
    void interceptAction(StandardActionManager::Type type, bool intercept = true);

    /**
     * Returns the list of collections that are currently selected.
     * The list is empty if no collection is currently selected.
     */
    Q_REQUIRED_RESULT Akonadi::Collection::List selectedCollections() const;

    /**
     * Returns the list of items that are currently selected.
     * The list is empty if no item is currently selected.
     */
    Q_REQUIRED_RESULT Akonadi::Item::List selectedItems() const;

    /**
     * @param names the list of names to set as collection properties page names
     */
    void setCollectionPropertiesPageNames(const QStringList &names);

Q_SIGNALS:
    /**
     * This signal is emitted whenever the action state has been updated.
     * In case you have special needs for changing the state of some actions,
     * connect to this signal and adjust the action state.
     */
    void actionStateUpdated();

private:
    //@cond PRIVATE
    class Private;
    std::unique_ptr<Private> const d;
    //@endcond
};
}

#endif
