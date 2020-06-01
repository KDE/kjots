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
#include <QSplitter>
#include <QStackedWidget>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTimer>
#include <QPrintDialog>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QDBusConnection>
#include <QMenu>
#include <QFileDialog>
#include <QAction>
#include <QIcon>
#include <QItemDelegate>
#include <QDebug>

// Akonadi
#include <Akonadi/Notes/NoteUtils>
#include <AkonadiCore/CollectionCreateJob>
#include <AkonadiCore/CollectionDeleteJob>
#include <AkonadiCore/ChangeRecorder>
#include <AkonadiCore/EntityDisplayAttribute>
#include <AkonadiCore/Item>
#include <AkonadiCore/ItemCreateJob>
#include <AkonadiCore/ItemModifyJob>
#include <AkonadiCore/ItemDeleteJob>
#include <AkonadiCore/ItemFetchScope>
#include <AkonadiCore/EntityOrderProxyModel>
#include <AkonadiWidgets/EntityTreeView>
#include <AkonadiWidgets/ETMViewStateSaver>
#include <AkonadiWidgets/ControlGui>

// Grantlee
#include <grantlee/template.h>
#include <grantlee/engine.h>
#include <grantlee/context.h>

// KDE
#include <KActionCollection>
#include <KBookmarkMenu>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSelectionProxyModel>
#include <KXMLGUIClient>
#include <KActionMenu>
#include <KRandom>
#include <KSharedConfig>
#include <KRun>
#include <KConfigDialog>

#include <KPIMTextEdit/RichTextComposerActions>
#include <KPIMTextEdit/RichTextEditorWidget>

// KMime
#include <KMime/Message>

// KJots
#include "kjotsbookmarks.h"
#include "kjotssortproxymodel.h"
#include "kjotsmodel.h"
#include "kjotsedit.h"
#include "kjotsconfigdlg.h"
#include "KJotsSettings.h"
#include "kjotsbrowser.h"
#include "noteshared/notelockattribute.h"
#include "noteshared/standardnoteactionmanager.h"

#include <memory>

using namespace Akonadi;
using namespace Grantlee;

KJotsWidget::KJotsWidget(QWidget *parent, KXMLGUIClient *xmlGuiClient, Qt::WindowFlags f)
    : QWidget(parent, f)
    , m_xmlGuiClient(xmlGuiClient)
{
    ControlGui::widgetNeedsAkonadi(this);

    // Grantlee
    m_loader = QSharedPointer<FileSystemTemplateLoader>(new FileSystemTemplateLoader());
    m_loader->setTemplateDirs(QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                        QStringLiteral("kjots/themes"),
                                                        QStandardPaths::LocateDirectory));
    m_loader->setTheme(QStringLiteral("default"));
    m_templateEngine = new Engine(this);
    m_templateEngine->addTemplateLoader(m_loader);

    // GUI & Actions
    setupGui();
    setupActions();
    // Models
    ItemFetchScope scope;
    scope.fetchFullPayload(true);   // Need to have full item when adding it to the internal data structure
    scope.fetchAttribute< EntityDisplayAttribute >();
    scope.fetchAttribute< NoteShared::NoteLockAttribute >();

    auto *monitor = new ChangeRecorder(this);
    monitor->fetchCollection(true);
    monitor->setItemFetchScope(scope);
    monitor->setCollectionMonitored(Collection::root());
    monitor->setMimeTypeMonitored(NoteUtils::noteMimeType());

    m_kjotsModel = new KJotsModel(monitor, this);

    m_sortProxyModel = new KJotsSortProxyModel(this);
    m_sortProxyModel->setSourceModel(m_kjotsModel);

    m_orderProxy = new EntityOrderProxyModel(this);
    m_orderProxy->setSourceModel(m_sortProxyModel);
    KConfigGroup cfg(KSharedConfig::openConfig(), "KJotsEntityOrder");
    m_orderProxy->setOrderConfig(cfg);

    m_treeview->setModel(m_orderProxy);
    m_actionManager->setCollectionSelectionModel(m_treeview->selectionModel());
    m_actionManager->setItemSelectionModel(m_treeview->selectionModel());

    m_collectionSelectionProxyModel = new KSelectionProxyModel(m_treeview->selectionModel(), this);
    m_collectionSelectionProxyModel->setSourceModel(m_treeview->model());

    connect(m_kjotsModel, &EntityTreeModel::modelAboutToBeReset, this, &KJotsWidget::saveState);
    connect(m_kjotsModel, &EntityTreeModel::modelReset, this, &KJotsWidget::restoreState);

    // TODO: handle dataChanged properly, i.e. if item was changed from outside
    connect(m_treeview->selectionModel(), &QItemSelectionModel::selectionChanged, this, &KJotsWidget::renderSelection);
    connect(m_treeview->selectionModel(), &QItemSelectionModel::selectionChanged, this, &KJotsWidget::updateMenu);
    connect(m_treeview->selectionModel(), &QItemSelectionModel::selectionChanged, this, &KJotsWidget::updateCaption);
    connect(m_treeview->model(), &QAbstractItemModel::dataChanged, this, &KJotsWidget::renderSelection);
    connect(m_treeview->model(), &QAbstractItemModel::dataChanged, this, &KJotsWidget::updateCaption);
    connect(m_editor, &KJotsEdit::documentModified, this, &KJotsWidget::updateCaption);

    QTimer::singleShot(0, this, &KJotsWidget::delayedInitialization);

    // Autosave timer
    m_autosaveTimer = new QTimer(this);
    updateConfiguration();
    connect(m_autosaveTimer, &QTimer::timeout, m_editor, &KJotsEdit::savePage);
    connect(m_treeview->selectionModel(), &QItemSelectionModel::selectionChanged, m_autosaveTimer, qOverload<>(&QTimer::start));

    restoreState();

    QDBusConnection::sessionBus().registerObject(QStringLiteral("/KJotsWidget"), this, QDBusConnection::ExportScriptableContents);
}

KJotsWidget::~KJotsWidget()
{
    saveState();
}

void KJotsWidget::setupGui()
{
    // Main horizontal layout
    auto *layout = new QHBoxLayout(this);
    layout->setMargin(0);

    // Splitter between collection view and browser / editor
    m_splitter = new QSplitter(this);
    m_splitter->setStretchFactor(1, 1);
    layout->addWidget(m_splitter);
    if (!KJotsSettings::splitterSizes().isEmpty()) {
        m_splitter->setSizes(KJotsSettings::splitterSizes());
    }

    // Collection view
    m_treeview = new EntityTreeView(m_xmlGuiClient, m_splitter);
    m_treeview->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_treeview->setEditTriggers(QAbstractItemView::DoubleClicked);

    // Make sure the editor gets focus again after naming a new book/page.
    connect(m_treeview->itemDelegate(), &QItemDelegate::closeEditor, this, [this](){
            activeEditor()->setFocus();
        });

    // Stacked widget containing editor & browser
    m_stackedWidget = new QStackedWidget(m_splitter);

    // Editor
    m_editor = new KJotsEdit(m_stackedWidget, m_xmlGuiClient->actionCollection());
    m_editorWidget = new KPIMTextEdit::RichTextEditorWidget(m_editor, m_stackedWidget);
    m_editor->setParent(m_editorWidget);
    m_stackedWidget->addWidget(m_editorWidget);
    connect(m_editor, &KJotsEdit::linkClicked, this, &KJotsWidget::openLink);
    // Browser
    m_browserWidget = new KJotsBrowserWidget(std::make_unique<KJotsBrowser>(m_kjotsModel, m_xmlGuiClient->actionCollection()),
                                             m_stackedWidget);
    m_stackedWidget->addWidget(m_browserWidget);
    m_stackedWidget->setCurrentWidget(m_browserWidget);
    connect(m_browserWidget->browser(), &KJotsBrowser::linkClicked, this, &KJotsWidget::openLink);
}

void KJotsWidget::setupActions()
{
    KActionCollection *actionCollection = m_xmlGuiClient->actionCollection();
    // Default Akonadi Notes actions
    m_actionManager = new StandardNoteActionManager(actionCollection, this);
    m_actionManager->createAllActions();

    actionCollection->setDefaultShortcut(m_actionManager->action(StandardActionManager::CreateCollection),
                                         QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_N));
    actionCollection->setDefaultShortcut(m_actionManager->action(StandardActionManager::DeleteCollections),
                                         QKeySequence(Qt::CTRL + Qt::Key_Delete));
    actionCollection->setDefaultShortcut(m_actionManager->action(StandardActionManager::DeleteItems),
                                         QKeySequence(Qt::CTRL + Qt::Key_Delete));


    QAction *action;
    // Standard actions
    KStandardAction::preferences(this, &KJotsWidget::configure, actionCollection);

    KStandardAction::save( m_editor, &KJotsEdit::savePage, actionCollection);

    action = KStandardAction::next(this, &KJotsWidget::nextPage, actionCollection);
    actionCollection->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::Key_PageDown));
    connect(this, &KJotsWidget::canGoNextPageChanged, action, &QAction::setEnabled);

    action = KStandardAction::prior(this, &KJotsWidget::prevPage, actionCollection);
    actionCollection->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::Key_PageUp));
    connect(this, &KJotsWidget::canGoPreviousPageChanged, action, &QAction::setEnabled);

    KStandardAction::renameFile(m_treeview, [this](){
            const QModelIndexList rows = m_treeview->selectionModel()->selectedRows();
            if (rows.size() != 1) {
                return;
            }
            m_treeview->edit(rows.first());
        }, actionCollection);

    action = KStandardAction::cut(m_editor, &KJotsEdit::cut, actionCollection);
    connect(m_editor, &KJotsEdit::copyAvailable, action, &QAction::setEnabled);
    action->setEnabled(false);

    action = KStandardAction::copy(this, [this](){
            activeEditor()->copy();
        }, actionCollection);
    connect(m_editor, &KJotsEdit::copyAvailable, action, &QAction::setEnabled);
    connect(m_browserWidget->browser(), &KJotsBrowser::copyAvailable, action, &QAction::setEnabled);
    action->setEnabled(false);

    KStandardAction::paste(m_editor, &KJotsEdit::paste, actionCollection);
    KStandardAction::undo(m_editor, &KJotsEdit::undo, actionCollection);
    KStandardAction::redo(m_editor, &KJotsEdit::redo, actionCollection);
    KStandardAction::selectAll(m_editor, &KJotsEdit::selectAll, actionCollection);

    KStandardAction::find(this, [this](){
            if (m_editorWidget->isVisible()) {
                m_editorWidget->slotFind();
            } else {
                m_browserWidget->slotFind();
            }
        }, actionCollection);
    KStandardAction::findNext(this, [this](){
            if (m_editorWidget->isVisible()) {
                m_editorWidget->slotFindNext();
            } else {
                m_browserWidget->slotFindNext();
            }
        }, actionCollection);
    KStandardAction::replace(this, [this](){
            if (m_editorWidget->isVisible()) {
                m_editorWidget->slotReplace();
            }
        }, actionCollection);

    KStandardAction::print(this, &KJotsWidget::printSelection, actionCollection);
    KStandardAction::printPreview(this, &KJotsWidget::printPreviewSelection, actionCollection);

    // Bookmarks actions
    auto *bookmarkMenu = actionCollection->add<KActionMenu>(QStringLiteral("bookmarks"));
    bookmarkMenu->setText(i18n("&Bookmarks"));
    auto *bookmarks = new KJotsBookmarks(m_treeview->selectionModel(), this);
    connect(bookmarks, &KJotsBookmarks::openLink, this, &KJotsWidget::openLink);
    auto *bmm = new KBookmarkMenu(
        KBookmarkManager::managerForFile(
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/kjots/bookmarks.xml"),
            QStringLiteral("kjots")),
        bookmarks, bookmarkMenu->menu());
    // "Add bookmark" and "make text bold" actions have conflicting shortcuts (ctrl + b)
    // Make add_bookmark use ctrl+shift+b to resolve that.
    action = bmm->addBookmarkAction();
    actionCollection->addAction(QStringLiteral("add_bookmark"), action);
    actionCollection->setDefaultShortcut(action, Qt::CTRL | Qt::SHIFT | Qt::Key_B);
    actionCollection->addAction(QStringLiteral("edit_bookmark"), bmm->editBookmarksAction());
    actionCollection->addAction(QStringLiteral("add_bookmarks_list"), bmm->bookmarkTabsAsFolderAction());

    // Export actions
    auto *exportMenu = actionCollection->add<KActionMenu>(QStringLiteral("save_to"));
    exportMenu->setText(i18n("Export"));
    exportMenu->setIcon(QIcon::fromTheme(QStringLiteral("document-export")));

    action = actionCollection->addAction(QStringLiteral("save_to_ascii"));
    action->setText(i18n("To Text File..."));
    action->setIcon(QIcon::fromTheme(QStringLiteral("text-plain")));
    connect(action, &QAction::triggered, this, [this](){
            exportSelection(QStringLiteral("plain_text"), QStringLiteral("template.txt"));
        });
    exportMenu->menu()->addAction(action);

    action = actionCollection->addAction(QStringLiteral("save_to_html"));
    action->setText(i18n("To HTML File..."));
    action->setIcon(QIcon::fromTheme(QStringLiteral("text-html")));
    connect(action, &QAction::triggered, this, [this](){
            exportSelection(QStringLiteral("default"), QStringLiteral("template.html"));
        });
    exportMenu->menu()->addAction(action);

    // Some extra actions
    action = actionCollection->addAction(QStringLiteral("go_next_book"));
    action->setText(i18n("Next Book"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("go-down")));
    actionCollection->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::Key_D));
    connect(action, &QAction::triggered, this, &KJotsWidget::nextBook);
    connect(this, &KJotsWidget::canGoNextBookChanged, action, &QAction::setEnabled);

    action = actionCollection->addAction(QStringLiteral("go_prev_book"));
    action->setText(i18n("Previous Book"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("go-up")));
    actionCollection->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_D));
    connect(action, &QAction::triggered, this, &KJotsWidget::prevBook);
    connect(this, &KJotsWidget::canGoPreviousBookChanged, action, &QAction::setEnabled);

    action = actionCollection->addAction(QStringLiteral("sort_children_alpha"));
    action->setText(i18n("Sort children alphabetically"));
    connect(action, &QAction::triggered, this, &KJotsWidget::actionSortChildrenAlpha);

    action = actionCollection->addAction(QStringLiteral("sort_children_by_date"));
    action->setText(i18n("Sort children by creation date"));
    connect(action, &QAction::triggered, this, &KJotsWidget::actionSortChildrenByDate);
}

void KJotsWidget::restoreState()
{
    auto *saver = new ETMViewStateSaver;
    saver->setView(m_treeview);
    KConfigGroup cfg(KSharedConfig::openConfig(), "TreeState");
    saver->restoreState(cfg);
}

void KJotsWidget::saveState()
{
    ETMViewStateSaver saver;
    saver.setView(m_treeview);
    KConfigGroup cfg(KSharedConfig::openConfig(), "TreeState");
    saver.saveState(cfg);
    cfg.sync();
}

void KJotsWidget::delayedInitialization()
{
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
    entryActions = { actionCollection->action(QString::fromLatin1(KStandardAction::name(KStandardAction::Find))),
                     actionCollection->action(QString::fromLatin1(KStandardAction::name(KStandardAction::Print))),
                     actionCollection->action(QString::fromLatin1(KStandardAction::name(KStandardAction::RenameFile))),
                     actionCollection->action(QStringLiteral("save_to")) };

    // Actions that are used only when a page is selected.
    pageActions = { actionCollection->action(QString::fromLatin1(KStandardAction::name(KStandardAction::Cut))),
                    actionCollection->action(QString::fromLatin1(KStandardAction::name(KStandardAction::Paste))),
                    actionCollection->action(QString::fromLatin1(KStandardAction::name(KStandardAction::Replace))),
                    actionCollection->action(QString::fromLatin1(KStandardAction::name(KStandardAction::Save))) };

    // Actions that are used only when a book is selected.
    bookActions = { actionCollection->action(QStringLiteral("sort_children_alpha")),
                    actionCollection->action(QStringLiteral("sort_children_by_date")) };

    // Actions that are used when multiple items are selected.
    multiselectionActions = { actionCollection->action(QString::fromLatin1(KStandardAction::name(KStandardAction::Find))),
                              actionCollection->action(QString::fromLatin1(KStandardAction::name(KStandardAction::Print))),
                              actionCollection->action(QStringLiteral("save_to")) };

    updateMenu();
}

inline QTextEdit *KJotsWidget::activeEditor()
{
    if (m_browserWidget->isVisible()) {
        return m_browserWidget->browser();
    } else {
        return m_editor;
    }
}

void KJotsWidget::updateMenu()
{
    QModelIndexList selection = m_treeview->selectionModel()->selectedRows();
    int selectionSize = selection.size();

    m_editor->setEnableActions(m_stackedWidget->currentWidget() == m_editorWidget);

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
            for (QAction *action : qAsConst(bookActions)) {
                action->setEnabled(true);
            }
        } else {
            for (QAction *action : qAsConst(pageActions)) {
                if (action->objectName() == QString::fromLatin1(name(KStandardAction::Cut))) {
                    action->setEnabled(activeEditor()->textCursor().hasSelection());
                } else {
                    action->setEnabled(true);
                }
            }
            for (QAction *action : qAsConst(bookActions)) {
                action->setEnabled(false);
            }
        }
    }
}

void KJotsWidget::configure()
{
    if (KConfigDialog::showDialog(QStringLiteral("kjotssettings"))) {
        return;
    }
    auto* dialog = new KConfigDialog(this, QStringLiteral("kjotssettings"), KJotsSettings::self());
    dialog->addPage(new KJotsConfigMisc(dialog), i18nc("@title:window config dialog page", "Misc"), QStringLiteral("preferences-other"));
    connect(dialog, &KConfigDialog::settingsChanged, this, &KJotsWidget::updateConfiguration);
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

QString KJotsWidget::renderSelectionTo(const QString &theme, const QString &templ)
{
    QList<QVariant> objectList;
    const int rows = m_collectionSelectionProxyModel->rowCount();
    const int column = 0;
    for (int row = 0; row < rows; ++row) {
        QModelIndex idx = m_collectionSelectionProxyModel->index(row, column, QModelIndex());
        objectList << idx.data(KJotsModel::GrantleeObjectRole);
    }
    QHash<QString, QVariant> hash = {{QStringLiteral("entities"), objectList},
                                     {QStringLiteral("i18n_TABLE_OF_CONTENTS"), i18nc("Header for 'Table of contents' section of rendered output", "Table of contents")}};
    Context c(hash);

    const QString currentTheme = m_loader->themeName();
    m_loader->setTheme(theme);
    Template t = m_templateEngine->loadByName(templ);
    const QString result = t->render(&c);
    m_loader->setTheme(currentTheme);
    return result;
}

QString KJotsWidget::renderSelectionToHtml()
{
    return renderSelectionTo(QStringLiteral("default"), QStringLiteral("template.html"));
}

void KJotsWidget::exportSelection(const QString &theme, const QString &templ)
{
    // TODO: dialog captions & etc
    QString filename = QFileDialog::getSaveFileName();
    if (filename.isEmpty()) {
        return;
    }
    QFile exportFile(filename);
    if (!exportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        KMessageBox::error(this, i18n("<qt>Could not open \"%1\" for writing</qt>", filename));
        return;
    }
    exportFile.write(renderSelectionTo(theme, templ).toUtf8());
    exportFile.close();
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
        m_loader->setTheme(QStringLiteral("default"));
        printDocument.setHtml(renderSelectionToHtml());
        m_loader->setTheme(currentTheme);
    }
    printDocument.print(printer);
}

void KJotsWidget::selectNext(int role, int step)
{
    QModelIndexList list = m_treeview->selectionModel()->selectedRows();
    Q_ASSERT(list.size() == 1);

    QModelIndex idx = list.at(0);

    const int column = idx.column();

    QModelIndex sibling = idx.sibling(idx.row() + step, column);
    while (sibling.isValid()) {
        if (sibling.data(role).toInt() >= 0) {
            m_treeview->selectionModel()->select(sibling, QItemSelectionModel::SelectCurrent);
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
    QModelIndexList list = m_treeview->selectionModel()->selectedRows();
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

    const int rows = m_collectionSelectionProxyModel->rowCount();
    // If the selection is a single page, present it for editing...
    if (rows == 1) {
        QModelIndex idx = m_collectionSelectionProxyModel->index(0, 0, QModelIndex());
        if (m_editor->setModelIndex(m_collectionSelectionProxyModel->mapToSource(idx))) {
            m_stackedWidget->setCurrentWidget(m_editorWidget);
            return;
        }
        // If something went wrong, we show user the browser
    }

    // ... Otherwise, render the selection read-only.
    m_browserWidget->browser()->setHtml(renderSelectionToHtml());
    m_stackedWidget->setCurrentWidget(m_browserWidget);
}

void KJotsWidget::updateCaption()
{
    const QModelIndexList selection = m_treeview->selectionModel()->selectedRows();
    QString caption;
    if (selection.size() == 1) {
        caption = KJotsModel::itemPath(selection.first());
        if (m_editor->modified()) {
            caption.append(QStringLiteral(" *"));
        }
    } else if (selection.size() > 1) {
        caption = i18nc("@title:window", "Multiple selection");
    }
    Q_EMIT captionChanged(caption);
}

bool KJotsWidget::queryClose()
{
    // Saving the current note
    // We cannot use async interface (i.e. ETM) here
    // because we need to abort the close if something went wrong
    if ((m_collectionSelectionProxyModel->rowCount() == 1) && (m_editor->document()->isModified())) {
        QModelIndex idx = m_collectionSelectionProxyModel->mapToSource(m_collectionSelectionProxyModel->index(0, 0, QModelIndex()));
        m_editor->prepareForSaving();
        auto job = new ItemModifyJob(KJotsModel::updateItem(idx, m_editor->document()));
        if (!job->exec()) {
            int res = KMessageBox::warningContinueCancelDetailed(this,
                                                                 i18n("Unable to save the note.\n"
                                                                      "You can save your note to a local file using the \"File - Export\" menu,\n"
                                                                      "otherwise you will lose your changes!\n"
                                                                      "Do you want to close anyways?"),
                                                                 i18n("Close Document"),
                                                                 KStandardGuiItem::quit(),
                                                                 KStandardGuiItem::cancel(),
                                                                 QString(),
                                                                 KMessageBox::Notify,
                                                                 i18n("Error message:\n"
                                                                      "%1", job->errorString()));
            if (res == KMessageBox::Cancel) {
                return false;
            }
        } else {
            // Saved successfully.
            // However, KJotsEdit will still catch focusOutEvent and try saving using async interface
            // (application will quit soon, so it doesn't really make much sense doing it)
            // Marking the document as saved explicitly it order to avoid it
            m_editor->document()->setModified(false);
        }

    }

    KJotsSettings::setSplitterSizes(m_splitter->sizes());
    KJotsSettings::self()->save();
    m_orderProxy->saveOrder();

    return true;
}

void KJotsWidget::actionSortChildrenAlpha()
{
    const QModelIndexList selection = m_treeview->selectionModel()->selectedRows();

    for (const QModelIndex &index : selection) {
        const QPersistentModelIndex persistent(index);
        m_sortProxyModel->sortChildrenAlphabetically(m_orderProxy->mapToSource(index));
        m_orderProxy->clearOrder(persistent);
    }
}

void KJotsWidget::actionSortChildrenByDate()
{
    const QModelIndexList selection = m_treeview->selectionModel()->selectedRows();

    for (const QModelIndex &index : selection) {
        const QPersistentModelIndex persistent(index);
        m_sortProxyModel->sortChildrenByCreationTime(m_orderProxy->mapToSource(index));
        m_orderProxy->clearOrder(persistent);
    }
}

void KJotsWidget::openLink(const QUrl &url)
{
    if (url.scheme() == QStringLiteral("akonadi")) {
        m_treeview->selectionModel()->select(KJotsModel::modelIndexForUrl(m_treeview->model(), url), QItemSelectionModel::ClearAndSelect);
    } else {
        new KRun(url, this);
    }
}
