/*
    This file is part of KJots.

    Copyright (C) 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
    Copyright (C) 2002, 2003 Aaron J. Seigo <aseigo@kde.org>
    Copyright (C) 2003 Stanislav Kljuhhin <crz@hot.ee>
    Copyright (C) 2005-2006 Jaison Lee <lee.jaison@gmail.com>
    Copyright (C) 2007-2009 Stephen Kelly <steveire@gmail.com>

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

#include "kjotswidget.h"

// Qt
#include <QHBoxLayout>
#include <QInputDialog>
#include <QSplitter>
#include <QStackedWidget>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTimer>
#include <QPrintDialog>
#include <QPainter>
#include <QPointer>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QAbstractTextDocumentLayout>
#include <QDBusConnection>
#include <QMenu>
#include <QFileDialog>
#include <QAction>
#include <QIcon>
#include <QItemDelegate>
#include <qdebug.h>

// Akonadi
#include <AkonadiCore/control.h>
#include <AkonadiCore/collectiondeletejob.h>
#include <AkonadiCore/collectioncreatejob.h>
#include <AkonadiCore/changerecorder.h>
#include <AkonadiCore/entitydisplayattribute.h>
#include <AkonadiWidgets/entitytreeview.h>
#include <AkonadiWidgets/etmviewstatesaver.h>
#include <AkonadiCore/item.h>
#include <AkonadiCore/ItemCreateJob>
#include <AkonadiCore/ItemDeleteJob>
#include <AkonadiCore/itemfetchjob.h>
#include <AkonadiCore/itemfetchscope.h>
#include <AkonadiCore/EntityOrderProxyModel>
#include <AkonadiWidgets/ControlGui>
#include <Akonadi/Notes/NoteUtils>

#include "akonadi_next/notecreatorandselector.h"

// Grantlee
#include <grantlee/template.h>
#include <grantlee/engine.h>
#include <grantlee/context.h>

// KDE
#include <KActionCollection>
#include <KBookmarkMenu>
#include <KFind>
#include <KFindDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <KReplaceDialog>
#include <kselectionproxymodel.h>
#include <KXMLGUIClient>
#include <KProcess>
#include <KActionMenu>
#include <krandom.h>
#include <KSharedConfig>
#include <KRun>

// KMime
#include <KMime/KMimeMessage>

// KJots
#include "kjotsbookmarks.h"
#include "kjotssortproxymodel.h"
#include "kjotsmodel.h"
#include "kjotsedit.h"
#include "kjotstreeview.h"
#include "kjotsconfigdlg.h"
#include "kjotsreplacenextdialog.h"
#include "KJotsSettings.h"
#include "kjotslockjob.h"
#include "kjotsbrowser.h"

#include "noteshared/notelockattribute.h"
#include "localresourcecreator.h"

#include <memory>

Q_DECLARE_METATYPE(QTextDocument *)
Q_DECLARE_METATYPE(QTextCursor)
using namespace Akonadi;
using namespace Grantlee;

KJotsWidget::KJotsWidget(QWidget *parent, KXMLGUIClient *xmlGuiClient, Qt::WindowFlags f)
    : QWidget(parent, f), m_xmlGuiClient(xmlGuiClient)
{
    Akonadi::ControlGui::widgetNeedsAkonadi(this);

    KConfigGroup config(KSharedConfig::openConfig(), "General");
    const bool autoCreate = config.readEntry("AutoCreateResourceOnStart", true);
    config.writeEntry("AutoCreateResourceOnStart", autoCreate);
    config.sync();
    if (autoCreate) {
        LocalResourceCreator *creator = new LocalResourceCreator(this);
        creator->createIfMissing();
    }

    m_splitter = new QSplitter(this);

    m_splitter->setStretchFactor(1, 1);
    // I think we can live without this
    //m_splitter->setOpaqueResize(KGlobalSettings::opaqueResize());

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);

    m_templateEngine = new Engine(this);
    // We don't have custom plugins, so this should not be needed
    //m_templateEngine->setPluginPaths(KStd.findDirs("lib", QString()));

    m_loader = QSharedPointer<FileSystemTemplateLoader>(new FileSystemTemplateLoader());
    m_loader->setTemplateDirs(QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                        QStringLiteral("kjots/themes"),
                                                        QStandardPaths::LocateDirectory));
    m_loader->setTheme(QLatin1String("default"));

    m_templateEngine->addTemplateLoader(m_loader);

    treeview = new KJotsTreeView(xmlGuiClient, m_splitter);

    ItemFetchScope scope;
    scope.fetchFullPayload(true);   // Need to have full item when adding it to the internal data structure
    scope.fetchAttribute< EntityDisplayAttribute >();
    scope.fetchAttribute< NoteShared::NoteLockAttribute >();

    ChangeRecorder *monitor = new ChangeRecorder(this);
    monitor->fetchCollection(true);
    monitor->setItemFetchScope(scope);
    monitor->setCollectionMonitored(Collection::root());
    monitor->setMimeTypeMonitored(Akonadi::NoteUtils::noteMimeType());

    m_kjotsModel = new KJotsModel(monitor, this);

    m_sortProxyModel = new KJotsSortProxyModel(this);
    m_sortProxyModel->setSourceModel(m_kjotsModel);

    m_orderProxy = new EntityOrderProxyModel(this);
    m_orderProxy->setSourceModel(m_sortProxyModel);

    KConfigGroup cfg(KSharedConfig::openConfig(), "KJotsEntityOrder");

    m_orderProxy->setOrderConfig(cfg);

    treeview->setModel(m_orderProxy);
    treeview->setSelectionMode(QAbstractItemView::ExtendedSelection);
    treeview->setEditTriggers(QAbstractItemView::DoubleClicked);

    selProxy = new KSelectionProxyModel(treeview->selectionModel(), this);
    selProxy->setSourceModel(treeview->model());

    // TODO: Write a QAbstractItemView subclass to render kjots selection.

    // TODO: handle dataChanged properly, i.e. if item was changed from outside
    connect(selProxy, &KSelectionProxyModel::dataChanged, this, &KJotsWidget::renderSelection);
    connect(selProxy, &KSelectionProxyModel::rowsInserted, this, &KJotsWidget::renderSelection);
    connect(selProxy, &KSelectionProxyModel::rowsRemoved, this, &KJotsWidget::renderSelection);

    stackedWidget = new QStackedWidget(m_splitter);

    KActionCollection *actionCollection = xmlGuiClient->actionCollection();

    editor = new KJotsEdit(stackedWidget);
    connect(editor, &KJotsEdit::linkClicked, this, &KJotsWidget::openLink);
    actionCollection->addActions(editor->createActions());
    stackedWidget->addWidget(editor);

    layout->addWidget(m_splitter);

    browser = new KJotsBrowser(stackedWidget);
    connect(browser, &KJotsBrowser::linkClicked, this, &KJotsWidget::openLink);
    stackedWidget->addWidget(browser);
    stackedWidget->setCurrentWidget(browser);

    QAction *action;

    action = actionCollection->addAction(QLatin1String("go_next_book"));
    action->setText(i18n("Next Book"));
    action->setIcon(QIcon::fromTheme(QLatin1String("go-down")));
    actionCollection->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::Key_D));
    connect(action, &QAction::triggered, this, &KJotsWidget::nextBook);
    connect(this, &KJotsWidget::canGoNextBookChanged, action, &QAction::setEnabled);

    action = actionCollection->addAction(QLatin1String("go_prev_book"));
    action->setText(i18n("Previous Book"));
    action->setIcon(QIcon::fromTheme(QLatin1String("go-up")));
    actionCollection->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_D));
    connect(action, &QAction::triggered, this, &KJotsWidget::prevBook);
    connect(this, &KJotsWidget::canGoPreviousBookChanged, action, &QAction::setEnabled);

    action = KStandardAction::next(this, &KJotsWidget::nextPage, actionCollection);
    actionCollection->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::Key_PageDown));
    connect(this, &KJotsWidget::canGoNextPageChanged, action, &QAction::setEnabled);

    action = KStandardAction::prior(this, &KJotsWidget::prevPage, actionCollection);
    actionCollection->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::Key_PageUp));
    connect(this, &KJotsWidget::canGoPreviousPageChanged, action, &QAction::setEnabled);

    action = actionCollection->addAction(QLatin1String("new_page"));
    action->setText(i18n("&New Page"));
    actionCollection->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::Key_N));
    action->setIcon(QIcon::fromTheme(QLatin1String("document-new")));
    connect(action, &QAction::triggered, this, &KJotsWidget::newPage);

    action = actionCollection->addAction(QLatin1String("new_book"));
    action->setText(i18n("New &Book..."));
    actionCollection->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_N));
    action->setIcon(QIcon::fromTheme(QLatin1String("address-book-new")));
    connect(action, &QAction::triggered, this, &KJotsWidget::newBook);

    action = actionCollection->addAction(QLatin1String("del_page"));
    action->setText(i18n("&Delete Page"));
    actionCollection->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::Key_Delete));
    action->setIcon(QIcon::fromTheme(QLatin1String("edit-delete-page")));
    connect(action, &QAction::triggered, this, &KJotsWidget::deletePage);

    action = actionCollection->addAction(QLatin1String("del_folder"));
    action->setText(i18n("Delete Boo&k"));
    actionCollection->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Delete));
    action->setIcon(QIcon::fromTheme(QLatin1String("edit-delete")));
    connect(action, &QAction::triggered, this, &KJotsWidget::deleteBook);

    action = actionCollection->addAction(QLatin1String("del_mult"));
    action->setText(i18n("Delete Selected"));
    action->setIcon(QIcon::fromTheme(QLatin1String("edit-delete")));
    connect(action, &QAction::triggered, this, &KJotsWidget::deleteMultiple);

    KStandardAction::save( editor, &KJotsEdit::savePage, actionCollection);

    action = actionCollection->addAction(QLatin1String("auto_bullet"));
    action->setText(i18n("Auto Bullets"));
    action->setIcon(QIcon::fromTheme(QLatin1String("format-list-unordered")));
    action->setCheckable(true);

    action = actionCollection->addAction(QLatin1String("auto_decimal"));
    action->setText(i18n("Auto Decimal List"));
    action->setIcon(QIcon::fromTheme(QLatin1String("format-list-ordered")));
    action->setCheckable(true);

    action = actionCollection->addAction(QLatin1String("manage_link"));
    action->setText(i18n("Link"));
    action->setIcon(QIcon::fromTheme(QLatin1String("insert-link")));

    action = actionCollection->addAction(QLatin1String("insert_image"));
    action->setText(i18n("Insert Image"));
    action->setIcon(QIcon::fromTheme(QLatin1String("insert-image")));

    action = actionCollection->addAction(QLatin1String("insert_checkmark"));
    action->setText(i18n("Insert Checkmark"));
    action->setIcon(QIcon::fromTheme(QLatin1String("checkmark")));
    action->setEnabled(false);

    KStandardAction::renameFile(treeview, &KJotsTreeView::renameEntry, actionCollection);

    action = actionCollection->addAction(QLatin1String("insert_date"));
    action->setText(i18n("Insert Date"));
    actionCollection->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_I));
    action->setIcon(QIcon::fromTheme(QLatin1String("view-calendar-time-spent")));

    action = actionCollection->addAction(QLatin1String("change_color"));
    action->setIcon(QIcon::fromTheme(QLatin1String("format-fill-color")));
    action->setText(i18n("Change Color..."));

    action = actionCollection->addAction(QLatin1String("copy_link_address"));
    action->setText(i18n("Copy Link Address"));

    action = actionCollection->addAction(QLatin1String("lock"));
    action->setText(i18n("Lock Selected"));
    action->setIcon(QIcon::fromTheme(QLatin1String("emblem-locked")));
    connect(action, &QAction::triggered, this, &KJotsWidget::actionLock);

    action = actionCollection->addAction(QLatin1String("unlock"));
    action->setText(i18n("Unlock Selected"));
    action->setIcon(QIcon::fromTheme(QLatin1String("emblem-unlocked")));
    connect(action, &QAction::triggered, this, &KJotsWidget::actionUnlock);

    action = actionCollection->addAction(QLatin1String("sort_children_alpha"));
    action->setText(i18n("Sort children alphabetically"));
    connect(action, &QAction::triggered, this, &KJotsWidget::actionSortChildrenAlpha);

    action = actionCollection->addAction(QLatin1String("sort_children_by_date"));
    action->setText(i18n("Sort children by creation date"));
    connect(action, &QAction::triggered, this, &KJotsWidget::actionSortChildrenByDate);

    action = KStandardAction::cut(editor, &KJotsEdit::cut, actionCollection);
    connect(editor, &KJotsEdit::copyAvailable, action, &QAction::setEnabled);
    action->setEnabled(false);

    action = KStandardAction::copy(this, &KJotsWidget::copy, actionCollection);
    connect(editor, &KJotsEdit::copyAvailable, action, &QAction::setEnabled);
    connect(browser, &KJotsBrowser::copyAvailable, action, &QAction::setEnabled);
    action->setEnabled(false);

    KStandardAction::paste(editor, &KJotsEdit::paste, actionCollection);

    KStandardAction::undo(editor, &KJotsEdit::undo, actionCollection);
    KStandardAction::redo(editor, &KJotsEdit::redo, actionCollection);
    KStandardAction::selectAll(editor, &KJotsEdit::selectAll, actionCollection);

    action = actionCollection->addAction(QLatin1String("copyIntoTitle"));
    action->setText(i18n("Copy &into Page Title"));
    actionCollection->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::Key_T));
    action->setIcon(QIcon::fromTheme(QLatin1String("edit-copy")));
    connect(action, &QAction::triggered, this, &KJotsWidget::copySelectionToTitle);
    connect(editor, &KJotsEdit::copyAvailable, action, &QAction::setEnabled);
    action->setEnabled(false);

    action = actionCollection->addAction(QLatin1String("paste_plain_text"));
    action->setText(i18nc("@action Paste the text in the clipboard without rich text formatting.", "Paste Plain Text"));
    connect(action, &QAction::triggered, editor, &KJotsEdit::pastePlainText);

    KStandardAction::preferences(this, &KJotsWidget::configure, actionCollection);

    bookmarkMenu = actionCollection->add<KActionMenu>(QLatin1String("bookmarks"));
    bookmarkMenu->setText(i18n("&Bookmarks"));
    KJotsBookmarks *bookmarks = new KJotsBookmarks(treeview);
    connect(bookmarks, &KJotsBookmarks::openLink, this, &KJotsWidget::openLink);
    KBookmarkMenu *bmm = new KBookmarkMenu(
        KBookmarkManager::managerForFile(
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/kjots/bookmarks.xml"),
            QStringLiteral("kjots")),
        bookmarks, bookmarkMenu->menu());

    // "Add bookmark" and "make text bold" actions have conflicting shortcuts (ctrl + b)
    // Make add_bookmark use ctrl+shift+b to resolve that.
    QAction *bm_action = bmm->addBookmarkAction();
    actionCollection->addAction(QStringLiteral("add_bookmark"), bm_action);
    actionCollection->setDefaultShortcut(bm_action, Qt::CTRL | Qt::SHIFT | Qt::Key_B);
    actionCollection->addAction(QStringLiteral("edit_bookmark"), bmm->editBookmarksAction());
    actionCollection->addAction(QStringLiteral("add_bookmarks_list"), bmm->bookmarkTabsAsFolderAction());


    KStandardAction::find(this, &KJotsWidget::onShowSearch, actionCollection);
    action = KStandardAction::findNext(this, &KJotsWidget::onRepeatSearch, actionCollection);
    action->setEnabled(false);
    KStandardAction::replace(this, &KJotsWidget::onShowReplace, actionCollection);

    action = actionCollection->addAction(QLatin1String("save_to"));
    action->setText(i18n("Rename..."));
    action->setIcon(QIcon::fromTheme(QLatin1String("edit-rename")));
    actionCollection->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::Key_M));

    KActionMenu *exportMenu = actionCollection->add<KActionMenu>(QLatin1String("save_to"));
    exportMenu->setText(i18n("Export"));
    exportMenu->setIcon(QIcon::fromTheme(QLatin1String("document-export")));

    action = actionCollection->addAction(QLatin1String("save_to_ascii"));
    action->setText(i18n("To Text File..."));
    action->setIcon(QIcon::fromTheme(QLatin1String("text-plain")));
    connect(action, &QAction::triggered, this, &KJotsWidget::exportSelectionToPlainText);
    exportMenu->menu()->addAction(action);

    action = actionCollection->addAction(QLatin1String("save_to_html"));
    action->setText(i18n("To HTML File..."));
    action->setIcon(QIcon::fromTheme(QLatin1String("text-html")));
    connect(action, &QAction::triggered, this, &KJotsWidget::exportSelectionToHtml);
    exportMenu->menu()->addAction(action);

    action = actionCollection->addAction(QLatin1String("save_to_book"));
    action->setText(i18n("To Book File..."));
    action->setIcon(QIcon::fromTheme(QLatin1String("x-office-address-book")));
    connect(action, &QAction::triggered, this, &KJotsWidget::exportSelectionToXml);
    exportMenu->menu()->addAction(action);

    KStandardAction::print(this, &KJotsWidget::printSelection, actionCollection);
    KStandardAction::printPreview(this, &KJotsWidget::printPreviewSelection, actionCollection);

    if (!KJotsSettings::splitterSizes().isEmpty()) {
        m_splitter->setSizes(KJotsSettings::splitterSizes());
    }

    QTimer::singleShot(0, this, &KJotsWidget::delayedInitialization);

    connect(treeview->selectionModel(), &QItemSelectionModel::selectionChanged, this, &KJotsWidget::updateMenu);
    connect(treeview->selectionModel(), &QItemSelectionModel::selectionChanged, this, &KJotsWidget::updateCaption);

    connect(m_kjotsModel, &Akonadi::EntityTreeModel::modelAboutToBeReset, this, &KJotsWidget::saveState);
    connect(m_kjotsModel, &Akonadi::EntityTreeModel::modelReset, this, &KJotsWidget::restoreState);

    restoreState();

    QDBusConnection::sessionBus().registerObject(QLatin1String("/KJotsWidget"), this, QDBusConnection::ExportScriptableContents);
}

KJotsWidget::~KJotsWidget()
{
    saveState();
}

void KJotsWidget::restoreState()
{
    ETMViewStateSaver *saver = new ETMViewStateSaver;
    saver->setView(treeview);
    KConfigGroup cfg(KSharedConfig::openConfig(), "TreeState");
    saver->restoreState(cfg);
}

void KJotsWidget::saveState()
{
    ETMViewStateSaver saver;
    saver.setView(treeview);
    KConfigGroup cfg(KSharedConfig::openConfig(), "TreeState");
    saver.saveState(cfg);
    cfg.sync();
}

void KJotsWidget::delayedInitialization()
{
    //TODO: Save previous searches in settings file?
    searchDialog = new KFindDialog(this, 0, QStringList(), false);
    QGridLayout *layout = new QGridLayout(searchDialog->findExtension());
    layout->setMargin(0);
    searchAllPages = new QCheckBox(i18n("Search all pages"), searchDialog->findExtension());
    layout->addWidget(searchAllPages, 0, 0);

    connect(searchDialog, &KFindDialog::okClicked, this, &KJotsWidget::onStartSearch);
    connect(searchDialog, &KFindDialog::cancelClicked, this, &KJotsWidget::onEndSearch);
    connect(treeview->selectionModel(), &QItemSelectionModel::selectionChanged, this, &KJotsWidget::onUpdateSearch);
    connect(searchDialog, &KFindDialog::optionsChanged, this, &KJotsWidget::onUpdateSearch);
    connect(searchAllPages, &QCheckBox::stateChanged, this, &KJotsWidget::onUpdateSearch);

    replaceDialog = new KReplaceDialog(this, 0, searchHistory, replaceHistory, false);
    QGridLayout *layout2 = new QGridLayout(replaceDialog->findExtension());
    layout2->setMargin(0);
    replaceAllPages = new QCheckBox(i18n("Search all pages"), replaceDialog->findExtension());
    layout2->addWidget(replaceAllPages, 0, 0);

    connect(replaceDialog, &KReplaceDialog::okClicked, this, &KJotsWidget::onStartReplace);
    connect(replaceDialog, &KReplaceDialog::cancelClicked, this, &KJotsWidget::onEndReplace);
    connect(replaceDialog, &KReplaceDialog::optionsChanged, this, &KJotsWidget::onUpdateReplace);
    connect(replaceAllPages, &QCheckBox::stateChanged, this, &KJotsWidget::onUpdateReplace);

    // Actions are enabled or disabled based on whether the selection is a single page, a single book
    // multiple selections, or no selection.
    //
    // The entryActions are enabled for all single pages and single books, and the multiselectionActions
    // are enabled when the user has made multiple selections.
    //
    // Some actions are in neither (eg, new book) and are available even when there is no selection.
    //
    // Some actions are in both, so that they are available for valid selections, but not available
    // for invalid selections (eg, print/find are disabled when there is no selection)

    KActionCollection *actionCollection = m_xmlGuiClient->actionCollection();

    // Actions for a single item selection.
    entryActions.insert(actionCollection->action(QLatin1String(KStandardAction::name(KStandardAction::Find))));
    entryActions.insert(actionCollection->action(QLatin1String(KStandardAction::name(KStandardAction::Print))));
    entryActions.insert(actionCollection->action(QLatin1String(KStandardAction::name(KStandardAction::RenameFile))));
    entryActions.insert(actionCollection->action(QLatin1String("change_color")));
    entryActions.insert(actionCollection->action(QLatin1String("save_to")));
    entryActions.insert(actionCollection->action(QLatin1String("copy_link_address")));

    // Actions that are used only when a page is selected.
    pageActions.insert(actionCollection->action(QLatin1String(KStandardAction::name(KStandardAction::Cut))));
    pageActions.insert(actionCollection->action(QLatin1String(KStandardAction::name(KStandardAction::Paste))));
    pageActions.insert(actionCollection->action(QLatin1String(KStandardAction::name(KStandardAction::Replace))));
    pageActions.insert(actionCollection->action(QLatin1String(KStandardAction::name(KStandardAction::Save))));
    pageActions.insert(actionCollection->action(QLatin1String("del_page")));
    pageActions.insert(actionCollection->action(QLatin1String("insert_date")));
    pageActions.insert(actionCollection->action(QLatin1String("auto_bullet")));
    pageActions.insert(actionCollection->action(QLatin1String("auto_decimal")));
    pageActions.insert(actionCollection->action(QLatin1String("manage_link")));
    pageActions.insert(actionCollection->action(QLatin1String("insert_checkmark")));

    // Actions that are used only when a book is selected.
    bookActions.insert(actionCollection->action(QLatin1String("save_to_book")));
    bookActions.insert(actionCollection->action(QLatin1String("del_folder")));
    bookActions.insert(actionCollection->action(QLatin1String("sort_children_alpha")));
    bookActions.insert(actionCollection->action(QLatin1String("sort_children_by_date")));

    // Actions that are used when multiple items are selected.
    multiselectionActions.insert(actionCollection->action(QLatin1String(KStandardAction::name(KStandardAction::Find))));
    multiselectionActions.insert(actionCollection->action(QLatin1String(KStandardAction::name(KStandardAction::Print))));
    multiselectionActions.insert(actionCollection->action(QLatin1String("del_mult")));
    multiselectionActions.insert(actionCollection->action(QLatin1String("save_to")));
    multiselectionActions.insert(actionCollection->action(QLatin1String("change_color")));

    m_autosaveTimer = new QTimer(this);
    updateConfiguration();

    connect(m_autosaveTimer, &QTimer::timeout, editor, &KJotsEdit::savePage);
    connect(treeview->selectionModel(), &QItemSelectionModel::selectionChanged, m_autosaveTimer, qOverload<>(&QTimer::start));

    treeview->delayedInitialization();
    editor->delayedInitialization(m_xmlGuiClient->actionCollection());
    browser->delayedInitialization();

    connect(treeview->itemDelegate(), &QItemDelegate::closeEditor, this, &KJotsWidget::bookshelfEditItemFinished);

    connect(editor, &KJotsEdit::currentCharFormatChanged, this, &KJotsWidget::currentCharFormatChanged);
    updateMenu();
}

void KJotsWidget::bookshelfEditItemFinished(QWidget *, QAbstractItemDelegate::EndEditHint)
{
    // Make sure the editor gets focus again after naming a new book/page.
    activeEditor()->setFocus();
}

void KJotsWidget::currentCharFormatChanged(const QTextCharFormat &fmt)
{
    QString selectedAnchor = fmt.anchorHref();
    if (selectedAnchor != activeAnchor) {
        activeAnchor = selectedAnchor;
        if (!selectedAnchor.isEmpty()) {
            QTextCursor c(editor->textCursor());
            editor->selectLinkText(&c);
            QString selectedText = c.selectedText();
            if (!selectedText.isEmpty()) {
                Q_EMIT activeAnchorChanged(selectedAnchor, selectedText);
            }
        } else {
            Q_EMIT activeAnchorChanged(QString(), QString());
        }
    }
}

inline QTextEdit *KJotsWidget::activeEditor()
{
    if (browser->isVisible()) {
        return browser;
    } else {
        return editor;
    }
}

void KJotsWidget::updateMenu()
{
    QModelIndexList selection = treeview->selectionModel()->selectedRows();
    int selectionSize = selection.size();

    if (!selectionSize) {
        // no (meaningful?) selection
        for (QAction *action : qAsConst(multiselectionActions)) {
            action->setEnabled(false);
        }
        for (QAction *action : qAsConst(entryActions)) {
            action->setEnabled(false);
        }
        for (QAction *action : qAsConst(bookActions)) {
            action->setEnabled(false);
        }
        for (QAction *action : qAsConst(pageActions)) {
            action->setEnabled(false);
        }
        editor->setActionsEnabled(false);
    } else if (selectionSize > 1) {
        for (QAction *action : qAsConst(entryActions)) {
            action->setEnabled(false);
        }
        for (QAction *action : qAsConst(bookActions)) {
            action->setEnabled(false);
        }
        for (QAction *action : qAsConst(pageActions)) {
            action->setEnabled(false);
        }
        for (QAction *action : qAsConst(multiselectionActions)) {
            action->setEnabled(true);
        }

        editor->setActionsEnabled(false);
    } else {

        for (QAction *action : qAsConst(multiselectionActions)) {
            action->setEnabled(false);
        }
        for (QAction *action : qAsConst(entryActions)) {
            action->setEnabled(true);
        }

        QModelIndex idx = selection.at(0);

        Collection col = idx.data(KJotsModel::CollectionRole).value<Collection>();
        if (col.isValid()) {
            for (QAction *action : qAsConst(pageActions)) {
                action->setEnabled(false);
            }
            const bool colIsRootCollection = (col.parentCollection() == Collection::root());
            for (QAction *action : qAsConst(bookActions)) {
                if (action->objectName() == QLatin1String("del_folder") && colIsRootCollection) {
                    action->setEnabled(false);
                } else {
                    action->setEnabled(true);
                }

            }

            editor->setActionsEnabled(false);
        } else {
            for (QAction *action : qAsConst(pageActions)) {
                if (action->objectName() == QLatin1String(name(KStandardAction::Cut))) {
                    action->setEnabled(activeEditor()->textCursor().hasSelection());
                } else {
                    action->setEnabled(true);
                }
            }
            for (QAction *action : qAsConst(bookActions)) {
                action->setEnabled(false);
            }
            editor->setActionsEnabled(true);
        }
    }
}

void KJotsWidget::copy()
{
    activeEditor()->copy();
}

void KJotsWidget::configure()
{
    // create a new preferences dialog...
    KJotsConfigDlg *dialog = new KJotsConfigDlg(i18n("Settings"), this);
    connect(dialog, qOverload<>(&KJotsConfigDlg::configCommitted), this, &KJotsWidget::updateConfiguration);
    dialog->show();
}

void KJotsWidget::updateConfiguration()
{
    if (KJotsSettings::autoSave()) {
        m_autosaveTimer->setInterval(KJotsSettings::autoSaveInterval() * 1000 * 60);
        m_autosaveTimer->start();
    } else {
        m_autosaveTimer->stop();
    }
}

void KJotsWidget::copySelectionToTitle()
{
    QString newTitle(editor->textCursor().selectedText());

    if (!newTitle.isEmpty()) {

        QModelIndexList rows = treeview->selectionModel()->selectedRows();

        if (rows.size() != 1) {
            return;
        }

        QModelIndex idx = rows.at(0);

        treeview->model()->setData(idx, newTitle);
    }
}

void KJotsWidget::deleteMultiple()
{
    const QModelIndexList selectedRows = treeview->selectionModel()->selectedRows();

    if (KMessageBox::questionYesNo(this,
                                   i18n("Do you really want to delete all selected books and pages?"),
                                   i18n("Delete?"), KStandardGuiItem::del(), KStandardGuiItem::cancel(),
                                   QString(), KMessageBox::Dangerous) != KMessageBox::Yes) {
        return;
    }

    for (const QModelIndex &index : selectedRows) {
        bool ok;
        qlonglong id = index.data(EntityTreeModel::ItemIdRole).toLongLong(&ok);
        Q_ASSERT(ok);
        if (id >= 0) {
            new ItemDeleteJob(Item(id), this);
        } else {
            id = index.data(EntityTreeModel::CollectionIdRole).toLongLong(&ok);
            Q_ASSERT(ok);
            if (id >= 0) {
                new CollectionDeleteJob(Collection(id), this);
            }
        }
    }
}

void KJotsWidget::deletePage()
{
    QModelIndexList selectedRows = treeview->selectionModel()->selectedRows();

    if (selectedRows.size() != 1) {
        return;
    }

    const QModelIndex idx = selectedRows.at(0);
    Item item = idx.data(EntityTreeModel::ItemRole).value<Item>();

    if (!item.isValid()) {
        return;
    }

    if (item.hasAttribute<NoteShared::NoteLockAttribute>()) {

        KMessageBox::information(topLevelWidget(),
                                 i18n("This page is locked. You can only delete it when you first unlock it."),
                                 i18n("Item is locked"));
        return;
    }

    if (KMessageBox::warningContinueCancel(topLevelWidget(),
                                           i18nc("remove the page, by title", "<qt>Are you sure you want to delete the page <strong>%1</strong>?</qt>", idx.data().toString()),
                                           i18n("Delete"), KStandardGuiItem::del(), KStandardGuiItem::cancel(), QLatin1String("DeletePageWarning")) == KMessageBox::Cancel) {
        return;
    }

    (void) new Akonadi::ItemDeleteJob(item, this);
}

void KJotsWidget::deleteBook()
{
    QModelIndexList selectedRows = treeview->selectionModel()->selectedRows();

    if (selectedRows.size() != 1) {
        return;
    }

    const QModelIndex idx = selectedRows.at(0);
    Collection col = idx.data(EntityTreeModel::CollectionRole).value<Collection>();

    if (!col.isValid()) {
        return;
    }

    if (col.parentCollection() == Collection::root()) {
        return;
    }

    if (col.hasAttribute<NoteShared::NoteLockAttribute>()) {

        KMessageBox::information(topLevelWidget(),
                                 i18n("This book is locked. You can only delete it when you first unlock it."),
                                 i18n("Item is locked"));
        return;
    }
    if (KMessageBox::warningContinueCancel(topLevelWidget(),
                                           i18nc("remove the book, by title", "<qt>Are you sure you want to delete the book <strong>%1</strong>?</qt>", idx.data().toString()),
                                           i18n("Delete"), KStandardGuiItem::del(), KStandardGuiItem::cancel(), QLatin1String("DeleteBookWarning")) == KMessageBox::Cancel) {
        return;
    }

    (void) new Akonadi::CollectionDeleteJob(col, this);
}

void KJotsWidget::newBook()
{
    QModelIndexList selectedRows = treeview->selectionModel()->selectedRows();

    if (selectedRows.size() != 1) {
        return;
    }

    Collection col = selectedRows.at(0).data(EntityTreeModel::CollectionRole).value<Collection>();

    if (!col.isValid()) {
        return;
    }

    Collection newCollection;
    newCollection.setParentCollection(col);

    QString title = i18nc("The default name for new books.", "New Book");
    newCollection.setName(KRandom::randomString(10));
    newCollection.setContentMimeTypes({Akonadi::Collection::mimeType(), Akonadi::NoteUtils::noteMimeType()});

    Akonadi::EntityDisplayAttribute *eda = new Akonadi::EntityDisplayAttribute();
    eda->setIconName(QLatin1String("x-office-address-book"));
    eda->setDisplayName(title);
    newCollection.addAttribute(eda);

    Akonadi::CollectionCreateJob *job = new Akonadi::CollectionCreateJob(newCollection);
    connect(job, &Akonadi::CollectionCreateJob::result, this, &KJotsWidget::newBookResult);
}

void KJotsWidget::newPage()
{
    QModelIndexList selectedRows = treeview->selectionModel()->selectedRows();

    if (selectedRows.size() != 1) {
        return;
    }

    Item item = selectedRows.at(0).data(EntityTreeModel::ItemRole).value<Item>();

    Collection col;
    if (item.isValid()) {
        col = selectedRows.at(0).data(EntityTreeModel::ParentCollectionRole).value<Collection>();
    } else {
        col = selectedRows.at(0).data(EntityTreeModel::CollectionRole).value<Collection>();
    }

    if (!col.isValid()) {
        return;
    }
    doCreateNewPage(col);
}

void KJotsWidget::doCreateNewPage(const Collection &collection)
{
    Akonotes::NoteCreatorAndSelector *creatorAndSelector = new Akonotes::NoteCreatorAndSelector(treeview->selectionModel());
    creatorAndSelector->createNote(collection);
}

void KJotsWidget::newPageResult(KJob *job)
{
    if (job->error()) {
        qDebug() << job->errorString();
    }
}

void KJotsWidget::newBookResult(KJob *job)
{
    if (job->error()) {
        qDebug() << job->errorString();
        return;
    }
    Akonadi::CollectionCreateJob *createJob = qobject_cast<Akonadi::CollectionCreateJob *>(job);
    if (!createJob) {
        return;
    }
    const Collection collection = createJob->collection();
    if (!collection.isValid()) {
        return;
    }

    doCreateNewPage(collection);
}

QString KJotsWidget::renderSelectionToHtml()
{
    QHash<QString, QVariant> hash;

    QList<QVariant> objectList;

    const int rows = selProxy->rowCount();
    const int column = 0;
    for (int row = 0; row < rows; ++row) {
        QModelIndex idx = selProxy->index(row, column, QModelIndex());

        QObject *obj = idx.data(KJotsModel::GrantleeObjectRole).value<QObject *>();
        KJotsEntity *kjotsEntity = qobject_cast<KJotsEntity *>(obj);
        kjotsEntity->setIndex(idx);
        objectList << QVariant::fromValue(static_cast<QObject *>(kjotsEntity));
    }

    hash.insert(QLatin1String("entities"), objectList);
    hash.insert(QLatin1String("i18n_TABLE_OF_CONTENTS"),
                i18nc("Header for 'Table of contents' section of rendered output", "Table of contents"));
    Context c(hash);

    Template t = m_templateEngine->loadByName(QLatin1String("template.html"));

    QString result = t->render(&c);

    // TODO: handle errors.
    return result;
}

QString KJotsWidget::renderSelectionToPlainText()
{
    QHash<QString, QVariant> hash;

    QList<QVariant> objectList;

    const int rows = selProxy->rowCount();
    const int column = 0;
    for (int row = 0; row < rows; ++row) {
        QModelIndex idx = selProxy->index(row, column, QModelIndex());

        QObject *obj = idx.data(KJotsModel::GrantleeObjectRole).value<QObject *>();
        KJotsEntity *kjotsEntity = qobject_cast<KJotsEntity *>(obj);
        kjotsEntity->setIndex(idx);
        objectList << QVariant::fromValue(static_cast<QObject *>(kjotsEntity));
    }

    hash.insert(QLatin1String("entities"), objectList);
    hash.insert(QLatin1String("i18n_TABLE_OF_CONTENTS"),
                i18nc("Header for 'Table of contents' section of rendered output", "Table of contents"));
    Context c(hash);

    Template t = m_templateEngine->loadByName(QLatin1String("template.txt"));

    QString result = t->render(&c);

    // TODO: handle errors.
    return result;
}

QString KJotsWidget::renderSelectionToXml()
{
    QHash<QString, QVariant> hash;

    QList<QVariant> objectList;

    const int rows = selProxy->rowCount();
    const int column = 0;
    for (int row = 0; row < rows; ++row) {
        QModelIndex idx = selProxy->index(row, column, QModelIndex());

        QObject *obj = idx.data(KJotsModel::GrantleeObjectRole).value<QObject *>();
        KJotsEntity *kjotsEntity = qobject_cast<KJotsEntity *>(obj);
        kjotsEntity->setIndex(idx);
        objectList << QVariant::fromValue(static_cast<QObject *>(kjotsEntity));
    }

    hash.insert(QLatin1String("entities"), objectList);
    Context c(hash);

    QString currentTheme = m_loader->themeName();
    m_loader->setTheme(QLatin1String("xml_output"));
    Template t = m_templateEngine->loadByName(QLatin1String("template.xml"));

    QString result = t->render(&c);

    m_loader->setTheme(currentTheme);
    return result;
}

QString KJotsWidget::getThemeFromUser()
{
    return QString();
#if 0
    bool ok;
    QString text = QInputDialog::getText(this, i18n("Change Theme"),
                                         tr("Theme name:"), QLineEdit::Normal,
                                         m_loader->themeName(), &ok);
    if (!ok || text.isEmpty()) {
        return QLatin1String("default");
    }

    return text;
#endif
}

void KJotsWidget::changeTheme()
{
#if 0
    m_loader->setTheme(getThemeFromUser());
    renderSelection();
#endif
}

void KJotsWidget::exportSelectionToHtml()
{
    QString currentTheme = m_loader->themeName();
    QString themeName = getThemeFromUser();
    if (themeName.isEmpty()) {
        themeName = QLatin1String("default");
    }
    m_loader->setTheme(themeName);

    QString filename = QFileDialog::getSaveFileName();
    if (!filename.isEmpty()) {
        QFile exportFile(filename);
        if (!exportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            m_loader->setTheme(currentTheme);
            KMessageBox::error(nullptr, i18n("<qt>Error opening internal file.</qt>"));
            return;
        }
        exportFile.write(renderSelectionToHtml().toUtf8());

        exportFile.close();
    }
    m_loader->setTheme(currentTheme);
}

void KJotsWidget::exportSelectionToPlainText()
{
    QString currentTheme = m_loader->themeName();

    m_loader->setTheme(QLatin1String("plain_text"));

    QString filename = QFileDialog::getSaveFileName();
    if (!filename.isEmpty()) {
        QFile exportFile(filename);
        if (!exportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            m_loader->setTheme(currentTheme);
            KMessageBox::error(nullptr, i18n("<qt>Error opening internal file.</qt>"));
            return;
        }
        exportFile.write(renderSelectionToPlainText().toUtf8());

        exportFile.close();
    }
    m_loader->setTheme(currentTheme);
}

void KJotsWidget::exportSelectionToXml()
{
    QString currentTheme = m_loader->themeName();

    m_loader->setTheme(QLatin1String("xml_output"));

    QString filename = QFileDialog::getSaveFileName();
    if (!filename.isEmpty()) {
        QFile exportFile(filename);
        if (!exportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            m_loader->setTheme(currentTheme);
            KMessageBox::error(nullptr, i18n("<qt>Error opening internal file.</qt>"));
            return;
        }
        exportFile.write(renderSelectionToXml().toUtf8());

        exportFile.close();
    }
    m_loader->setTheme(currentTheme);
}

std::unique_ptr<QPrinter> KJotsWidget::setupPrinter(QPrinter::PrinterMode mode)
{
    auto printer = std::make_unique<QPrinter>(mode);
    printer->setDocName(QStringLiteral("KJots_Print"));
    printer->setCreator(QStringLiteral("KJots"));
    if (!activeEditor()->textCursor().selection().isEmpty()) {
        printer->setPrintRange(QPrinter::Selection);
    }
    return printer;
}

void KJotsWidget::printPreviewSelection()
{
    auto printer = setupPrinter(QPrinter::ScreenResolution);
    QPrintPreviewDialog previewdlg(printer.get(), this);
    connect(&previewdlg, &QPrintPreviewDialog::paintRequested, this, &KJotsWidget::print);
    previewdlg.exec();
}

void KJotsWidget::printSelection()
{
    auto printer = setupPrinter(QPrinter::HighResolution);
    QPrintDialog printDialog(printer.get(), this);
    if (printDialog.exec() == QDialog::Accepted) {
        print(printer.get());
    }
}

void KJotsWidget::print(QPrinter *printer)
{
    QTextDocument printDocument;
    if (printer->printRange() == QPrinter::Selection) {
        printDocument.setHtml(activeEditor()->textCursor().selection().toHtml());
    } else {
        QString currentTheme = m_loader->themeName();
        m_loader->setTheme(QLatin1String("default"));
        printDocument.setHtml(renderSelectionToHtml());
        m_loader->setTheme(currentTheme);
    }
    printDocument.print(printer);
}

void KJotsWidget::selectNext(int role, int step)
{
    QModelIndexList list = treeview->selectionModel()->selectedRows();
    Q_ASSERT(list.size() == 1);

    QModelIndex idx = list.at(0);

    const int column = idx.column();

    QModelIndex sibling = idx.sibling(idx.row() + step, column);
    while (sibling.isValid()) {
        if (sibling.data(role).toInt() >= 0) {
            treeview->selectionModel()->select(sibling, QItemSelectionModel::SelectCurrent);
            return;
        }
        sibling = sibling.sibling(sibling.row() + step, column);
    }
    qWarning() << "No valid selection";
}

void KJotsWidget::nextBook()
{
    return selectNext(EntityTreeModel::CollectionIdRole, 1);
}

void KJotsWidget::nextPage()
{
    return selectNext(EntityTreeModel::ItemIdRole, 1);
}

void KJotsWidget::prevBook()
{
    return selectNext(EntityTreeModel::CollectionIdRole, -1);
}

void KJotsWidget::prevPage()
{
    return selectNext(EntityTreeModel::ItemIdRole, -1);
}

bool KJotsWidget::canGo(int role, int step) const
{
    QModelIndexList list = treeview->selectionModel()->selectedRows();
    if (list.size() != 1) {
        return false;
    }

    QModelIndex currentIdx = list.at(0);

    const int column = currentIdx.column();

    Q_ASSERT(currentIdx.isValid());

    QModelIndex sibling = currentIdx.sibling(currentIdx.row() + step, column);

    while (sibling.isValid() && sibling != currentIdx) {
        if (sibling.data(role).toInt() >= 0) {
            return true;
        }

        sibling = sibling.sibling(sibling.row() + step, column);
    }

    return false;
}

bool KJotsWidget::canGoNextPage() const
{
    return canGo(EntityTreeModel::ItemIdRole, 1);
}

bool KJotsWidget::canGoPreviousPage() const
{
    return canGo(EntityTreeModel::ItemIdRole, -1);
}

bool KJotsWidget::canGoNextBook() const
{
    return canGo(EntityTreeModel::CollectionIdRole, 1);
}

bool KJotsWidget::canGoPreviousBook() const
{
    return canGo(EntityTreeModel::CollectionIdRole, -1);
}

void KJotsWidget::renderSelection()
{
    Q_EMIT canGoNextBookChanged(canGoPreviousBook());
    Q_EMIT canGoNextPageChanged(canGoNextPage());
    Q_EMIT canGoPreviousBookChanged(canGoPreviousBook());
    Q_EMIT canGoPreviousPageChanged(canGoPreviousPage());

    const int rows = selProxy->rowCount();
    // If the selection is a single page, present it for editing...
    if (rows == 1) {
        QModelIndex idx = selProxy->index(0, 0, QModelIndex());
        if (editor->setModelIndex(selProxy->mapToSource(idx))) {
            stackedWidget->setCurrentWidget(editor);
            return;
        }
        // If something went wrong, we show user the browser
    }

    // ... Otherwise, render the selection read-only.
    browser->setHtml(renderSelectionToHtml());
    stackedWidget->setCurrentWidget(browser);
}

/*!
  Shows the search dialog when "Find" is selected.
*/
void KJotsWidget::onShowSearch()
{
    onUpdateSearch();

    QTextEdit *browserOrEditor = activeEditor();

    if (browserOrEditor->textCursor().hasSelection()) {
        searchDialog->setHasSelection(true);
        long dialogOptions = searchDialog->options();
        dialogOptions |= KFind::SelectedText;
        searchDialog->setOptions(dialogOptions);
    } else {
        searchDialog->setHasSelection(false);
    }

    searchDialog->setFindHistory(searchHistory);
    searchDialog->show();
    onUpdateSearch();
}

/*!
    Updates the search dialog if the user is switching selections while it is open.
*/
void KJotsWidget::onUpdateSearch()
{
    if (searchDialog->isVisible()) {
        long searchOptions = searchDialog->options();
        if (searchOptions & KFind::SelectedText) {
            searchAllPages->setCheckState(Qt::Unchecked);
            searchAllPages->setEnabled(false);
        } else {
            searchAllPages->setEnabled(true);
        }

        if (searchAllPages->checkState() == Qt::Checked) {
            searchOptions &= ~KFind::SelectedText;
            searchDialog->setOptions(searchOptions);
            searchDialog->setHasSelection(false);
        } else {
            if (activeEditor()->textCursor().hasSelection()) {
                searchDialog->setHasSelection(true);
            }
        }

        if (activeEditor()->textCursor().hasSelection()) {
            if (searchAllPages->checkState() == Qt::Unchecked) {
                searchDialog->setHasSelection(true);
            }
        } else {
            searchOptions &= ~KFind::SelectedText;
            searchDialog->setOptions(searchOptions);
            searchDialog->setHasSelection(false);
        }
    }
}

/*!
    Called when the user presses OK in the search dialog.
*/
void KJotsWidget::onStartSearch()
{
    QString searchPattern = searchDialog->pattern();
    if (!searchHistory.contains(searchPattern)) {
        searchHistory.prepend(searchPattern);
    }

    QTextEdit *browserOrEditor = activeEditor();
    QTextCursor cursor = browserOrEditor->textCursor();

    long searchOptions = searchDialog->options();
    if (searchOptions & KFind::FromCursor) {
        searchPos = cursor.position();
        searchBeginPos = 0;
        cursor.movePosition(QTextCursor::End);
        searchEndPos = cursor.position();
    } else {
        if (searchOptions & KFind::SelectedText) {
            searchBeginPos = cursor.selectionStart();
            searchEndPos = cursor.selectionEnd();
        } else {
            searchBeginPos = 0;
            cursor.movePosition(QTextCursor::End);
            searchEndPos = cursor.position();
        }

        if (searchOptions & KFind::FindBackwards) {
            searchPos = searchEndPos;
        } else {
            searchPos = searchBeginPos;
        }
    }

    m_xmlGuiClient->actionCollection()->action(QLatin1String(KStandardAction::name(KStandardAction::FindNext)))->setEnabled(true);

    onRepeatSearch();
}

/*!
    Called when user chooses "Find Next"
*/
void KJotsWidget::onRepeatSearch()
{
    if (search(false) == 0) {
        KMessageBox::sorry(nullptr, i18n("<qt>No matches found.</qt>"));
        m_xmlGuiClient->actionCollection()->action(QLatin1String(KStandardAction::name(KStandardAction::FindNext)))->setEnabled(false);
    }
}

/*!
    Called when user presses Cancel in find dialog.
*/
void KJotsWidget::onEndSearch()
{
    m_xmlGuiClient->actionCollection()->action(QLatin1String(KStandardAction::name(KStandardAction::FindNext)))->setEnabled(false);
}

/*!
    Shows the replace dialog when "Replace" is selected.
*/
void KJotsWidget::onShowReplace()
{
    Q_ASSERT(editor->isVisible());

    if (editor->textCursor().hasSelection()) {
        replaceDialog->setHasSelection(true);
        long dialogOptions = replaceDialog->options();
        dialogOptions |= KFind::SelectedText;
        replaceDialog->setOptions(dialogOptions);
    } else {
        replaceDialog->setHasSelection(false);
    }

    replaceDialog->setFindHistory(searchHistory);
    replaceDialog->setReplacementHistory(replaceHistory);
    replaceDialog->show();
    onUpdateReplace();
}

/*!
    Updates the replace dialog if the user is switching selections while it is open.
*/
void KJotsWidget::onUpdateReplace()
{
    if (replaceDialog->isVisible()) {
        long replaceOptions = replaceDialog->options();
        if (replaceOptions & KFind::SelectedText) {
            replaceAllPages->setCheckState(Qt::Unchecked);
            replaceAllPages->setEnabled(false);
        } else {
            replaceAllPages->setEnabled(true);
        }

        if (replaceAllPages->checkState() == Qt::Checked) {
            replaceOptions &= ~KFind::SelectedText;
            replaceDialog->setOptions(replaceOptions);
            replaceDialog->setHasSelection(false);
        } else {
            if (activeEditor()->textCursor().hasSelection()) {
                replaceDialog->setHasSelection(true);
            }
        }
    }
}

/*!
    Called when the user presses OK in the replace dialog.
*/
void KJotsWidget::onStartReplace()
{
    QString searchPattern = replaceDialog->pattern();
    if (!searchHistory.contains(searchPattern)) {
        searchHistory.prepend(searchPattern);
    }

    QString replacePattern = replaceDialog->replacement();
    if (!replaceHistory.contains(replacePattern)) {
        replaceHistory.prepend(replacePattern);
    }

    QTextCursor cursor = editor->textCursor();

    long replaceOptions = replaceDialog->options();
    if (replaceOptions & KFind::FromCursor) {
        replacePos = cursor.position();
        replaceBeginPos = 0;
        cursor.movePosition(QTextCursor::End);
        replaceEndPos = cursor.position();
    } else {
        if (replaceOptions & KFind::SelectedText) {
            replaceBeginPos = cursor.selectionStart();
            replaceEndPos = cursor.selectionEnd();
        } else {
            replaceBeginPos = 0;
            cursor.movePosition(QTextCursor::End);
            replaceEndPos = cursor.position();
        }

        if (replaceOptions & KFind::FindBackwards) {
            replacePos = replaceEndPos;
        } else {
            replacePos = replaceBeginPos;
        }
    }

    replaceStartPage = treeview->selectionModel()->selectedRows().first();

    //allow KReplaceDialog to exit so the user can see.
    QTimer::singleShot(0, this, &KJotsWidget::onRepeatReplace);
}

/*!
    Only called after onStartReplace. Kept the name scheme for consistancy.
*/
void KJotsWidget::onRepeatReplace()
{
    KJotsReplaceNextDialog *dlg = nullptr;

    QString searchPattern = replaceDialog->pattern();
    QString replacePattern = replaceDialog->replacement();
    int found = 0;
    int replaced = 0;

    long replaceOptions = replaceDialog->options();
    if (replaceOptions & KReplaceDialog::PromptOnReplace) {
        dlg = new KJotsReplaceNextDialog(this);
    }

    while (true) {
    if (!search(true))
        {
            break;
        }

        QTextCursor cursor = editor->textCursor();
        if (!cursor.hasSelection())
        {
            break;
        } else {
            ++found;
        }

        QString replacementText = replacePattern;
        if (replaceOptions & KReplaceDialog::BackReference)
        {
            QRegExp regExp(searchPattern, (replaceOptions & Qt::CaseSensitive) ?
            Qt::CaseSensitive : Qt::CaseInsensitive, QRegExp::RegExp2);
            regExp.indexIn(cursor.selectedText());
            int capCount = regExp.captureCount();
            for (int i = 0; i <= capCount; ++i) {
                QString c = QString::fromLatin1("\\%1").arg(i);
                replacementText.replace(c, regExp.cap(i));
            }
        }

        if (replaceOptions & KReplaceDialog::PromptOnReplace)
        {
            dlg->setLabel(cursor.selectedText(), replacementText);

            if (!dlg->exec()) {
                break;
            }

            if (dlg->answer() != KJotsReplaceNextDialog::Skip) {
                cursor.insertText(replacementText);
                editor->setTextCursor(cursor);
                ++replaced;
            }

            if (dlg->answer() == KJotsReplaceNextDialog::All) {
                replaceOptions |= ~KReplaceDialog::PromptOnReplace;
            }
        } else {
            cursor.insertText(replacementText);
            editor->setTextCursor(cursor);
            ++replaced;
        }
    }

    if (replaced == found) {
        KMessageBox::information(nullptr, i18np("<qt>Replaced 1 occurrence.</qt>", "<qt>Replaced %1 occurrences.</qt>", replaced));
    } else if (replaced < found) {
        KMessageBox::information(nullptr,
                                 i18np("<qt>Replaced %2 of 1 occurrence.</qt>", "<qt>Replaced %2 of %1 occurrences.</qt>", found, replaced));
    }

    if (dlg) {
        delete dlg;
    }
}

/*!
    Called when user presses Cancel in replace dialog. Just a placeholder for now.
*/
void KJotsWidget::onEndReplace()
{
}

/*!
    Searches for the given pattern, with the given options. This is huge and
    unwieldly function, but the operation we're performing is huge and unwieldly.
*/
int KJotsWidget::search(bool replacing)
{
    int rc = 0;
    int *beginPos = replacing ? &replaceBeginPos : &searchBeginPos;
    int *endPos = replacing ? &replaceEndPos : &searchEndPos;
    long options = replacing ? replaceDialog->options() : searchDialog->options();
    QString pattern = replacing ? replaceDialog->pattern() : searchDialog->pattern();
    int *curPos = replacing ? &replacePos : &searchPos;

    QModelIndex startPage = replacing ? replaceStartPage : treeview->selectionModel()->selectedRows().first();

    bool allPages = false;
    QCheckBox *box = replacing ? replaceAllPages : searchAllPages;
    if (box->isEnabled() && box->checkState() == Qt::Checked) {
        allPages = true;
    }

    QTextDocument::FindFlags findFlags;
    if (options & Qt::CaseSensitive) {
        findFlags |= QTextDocument::FindCaseSensitively;
    }

    if (options & KFind::WholeWordsOnly) {
        findFlags |= QTextDocument::FindWholeWords;
    }

    if (options & KFind::FindBackwards) {
        findFlags |= QTextDocument::FindBackward;
    }

    // We will find a match or return 0
    int attempts = 0;
    while (true) {
    ++attempts;

    QTextEdit *browserOrEditor = activeEditor();
        QTextDocument *theDoc = browserOrEditor->document();

        QTextCursor cursor;
        if (options & KFind::RegularExpression)
        {
            QRegExp regExp(pattern, (options & Qt::CaseSensitive) ?
            Qt::CaseSensitive : Qt::CaseInsensitive, QRegExp::RegExp2);
            cursor = theDoc->find(regExp, *curPos, findFlags);
        } else {
            cursor = theDoc->find(pattern, *curPos, findFlags);
        }

        if (cursor.hasSelection())
        {
            if (cursor.selectionStart() >= *beginPos && cursor.selectionEnd() <= *endPos) {
                browserOrEditor->setTextCursor(cursor);
                browserOrEditor->ensureCursorVisible();
                *curPos = (options & KFind::FindBackwards) ?
                cursor.selectionStart() : cursor.selectionEnd();
                rc = 1;
                break;
            }
        }

        //No match. Determine what to do next.

        if (replacing && !(options & KFind::FromCursor) && !allPages)
        {
            break;
        }

        if ((options & KFind::FromCursor) && !allPages)
        {
            if (KMessageBox::questionYesNo(this,
                                           i18n("<qt>End of search area reached. Do you want to wrap around and continue?</qt>")) ==
                    KMessageBox::No) {
                rc = 3;
                break;
            }
        }

        if (allPages)
        {
            if (options & KFind::FindBackwards) {
                if (canGoPreviousPage()) {
                    prevPage();
                }
            } else {
                if (canGoNextPage()) {
                    nextPage();
                }
            }

            if (startPage == treeview->selectionModel()->selectedRows().first()) {
                rc = 0;
                break;
            }

            *beginPos = 0;
            cursor = editor->textCursor();
            cursor.movePosition(QTextCursor::End);
            *endPos = cursor.position();
            *curPos = (options & KFind::FindBackwards) ? *endPos : *beginPos;
            continue;
        }

        // By now, we should have figured out what to do. In all remaining cases we
        // will automatically loop and try to "find next" from the top/bottom, because
        // I like this behavior the best.
        if (attempts <= 1)
        {
            *curPos = (options & KFind::FindBackwards) ? *endPos : *beginPos;
        } else {
            // We've already tried the loop and failed to find anything. Bail.
            rc = 0;
            break;
        }
    }

    return rc;
}

void KJotsWidget::updateCaption()
{
    Q_EMIT captionChanged(treeview->captionForSelection(QLatin1String(" / ")));
}

void KJotsWidget::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    QModelIndexList rows = treeview->selectionModel()->selectedRows();

    if (rows.size() != 1) {
        return;
    }

    QItemSelection changed(topLeft, bottomRight);
    if (changed.contains(rows.first())) {
        Q_EMIT captionChanged(treeview->captionForSelection(QLatin1String(" / ")));
    }
}

bool KJotsWidget::queryClose()
{
    KJotsSettings::setSplitterSizes(m_splitter->sizes());
    KJotsSettings::self()->save();
    m_orderProxy->saveOrder();
    // TODO: we better wait for a result here!
    editor->savePage();
    return true;
}

void KJotsWidget::actionLock()
{
    const QModelIndexList selection = treeview->selectionModel()->selectedRows();

    if (selection.isEmpty()) {
        return;
    }

    Collection::List collections;
    Item::List items;
    for (const QModelIndex &idx : selection) {
        Collection col = idx.data(EntityTreeModel::CollectionRole).value<Collection>();
        if (col.isValid()) {
            collections << col;
        } else {
            Item item = idx.data(EntityTreeModel::ItemRole).value<Item>();
            if (item.isValid()) {
                items << item;
            }
        }
    }
    if (collections.isEmpty() && items.isEmpty()) {
        return;
    }

    new KJotsLockJob(collections, items, this);
}

void KJotsWidget::actionUnlock()
{
    const QModelIndexList selection = treeview->selectionModel()->selectedRows();

    if (selection.isEmpty()) {
        return;
    }

    Collection::List collections;
    Item::List items;
    for (const QModelIndex &idx : selection) {
        Collection col = idx.data(EntityTreeModel::CollectionRole).value<Collection>();
        if (col.isValid()) {
            collections << col;
        } else {
            Item item = idx.data(EntityTreeModel::ItemRole).value<Item>();
            if (item.isValid()) {
                items << item;
            }
        }
    }
    if (collections.isEmpty() && items.isEmpty()) {
        return;
    }

    new KJotsLockJob(collections, items, KJotsLockJob::UnlockJob, this);
}

void KJotsWidget::actionSortChildrenAlpha()
{
    const QModelIndexList selection = treeview->selectionModel()->selectedRows();

    for (const QModelIndex &index : selection) {
        const QPersistentModelIndex persistent(index);
        m_sortProxyModel->sortChildrenAlphabetically(m_orderProxy->mapToSource(index));
        m_orderProxy->clearOrder(persistent);
    }
}

void KJotsWidget::actionSortChildrenByDate()
{
    const QModelIndexList selection = treeview->selectionModel()->selectedRows();

    for (const QModelIndex &index : selection) {
        const QPersistentModelIndex persistent(index);
        m_sortProxyModel->sortChildrenByCreationTime(m_orderProxy->mapToSource(index));
        m_orderProxy->clearOrder(persistent);
    }
}

void KJotsWidget::openLink(const QUrl &url)
{
    if (url.scheme() == QStringLiteral("akonadi")) {
        treeview->selectionModel()->select(KJotsModel::modelIndexForUrl(treeview->model(), url), QItemSelectionModel::ClearAndSelect);
    } else {
        new KRun(url, this);
    }
}
