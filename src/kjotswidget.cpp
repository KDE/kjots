/*
    This file is part of KJots.

    SPDX-FileCopyrightText: 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
                  2002, 2003 Aaron J. Seigo <aseigo@kde.org>
                  2003 Stanislav Kljuhhin <crz@hot.ee>
                  2005-2006 Jaison Lee <lee.jaison@gmail.com>
                  2007-2009 Stephen Kelly <steveire@gmail.com>
                  2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
#include <QHeaderView>
#include <QDebug>
#include <QActionGroup>

// Akonadi
#include <akonadi_version.h>
#include <Akonadi/NoteUtils>
#include <Akonadi/AttributeFactory>
#include <Akonadi/CollectionCreateJob>
#include <Akonadi/CollectionDeleteJob>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/EntityMimeTypeFilterModel>
#include <Akonadi/Item>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/EntityOrderProxyModel>
#include <Akonadi/EntityTreeView>
#include <Akonadi/ETMViewStateSaver>
#include <Akonadi/ControlGui>

#include <KTextTemplate/Engine>
#include <KTextTemplate/Template>
#include <KTextTemplate/Context>

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
#include <KConfigDialog>
#include <KIO/OpenUrlJob>

#include <KPIMTextEdit/RichTextComposerActions>
#include <TextCustomEditor/RichTextEditorWidget>


// KMime
#include <KMime/Message>

// KJots
#include "uistatesaver.h"
#include "kjotsbookmarks.h"
#include "kjotsmodel.h"
#include "kjotsedit.h"
#include "kjotsconfigdlg.h"
#include "KJotsSettings.h"
#include "kjotsbrowser.h"
#include "noteshared/notelockattribute.h"
#include "noteshared/notepinattribute.h"
#include "noteshared/standardnoteactionmanager.h"
#include "notesortproxymodel.h"

#include <memory>

using namespace Akonadi;
using namespace KTextTemplate;
KJotsWidget::KJotsWidget(QWidget *parent, KXMLGUIClient *xmlGuiClient, Qt::WindowFlags f)
    : QWidget(parent, f)
    , m_xmlGuiClient(xmlGuiClient)
{
    ControlGui::widgetNeedsAkonadi(this);

    Akonadi::AttributeFactory::registerAttribute<NoteShared::NoteLockAttribute>();
    Akonadi::AttributeFactory::registerAttribute<NoteShared::NotePinAttribute>();

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
    scope.fetchAttribute<EntityDisplayAttribute>();
    scope.fetchAttribute<NoteShared::NoteLockAttribute>();
    scope.fetchAttribute<NoteShared::NotePinAttribute>();

    auto monitor = new ChangeRecorder(this);
    monitor->fetchCollection(true);
    monitor->setItemFetchScope(scope);
    monitor->setCollectionMonitored(Collection::root());
    monitor->setMimeTypeMonitored(NoteUtils::noteMimeType());

    m_kjotsModel = new KJotsModel(monitor, this);

    m_browserWidget->browser()->setModel(m_kjotsModel);

    m_collectionModel = new EntityMimeTypeFilterModel(this);
    m_collectionModel->setSourceModel(m_kjotsModel);
    m_collectionModel->addMimeTypeInclusionFilter(Collection::mimeType());
    m_collectionModel->setHeaderGroup(EntityTreeModel::CollectionTreeHeaders);
    m_collectionModel->setDynamicSortFilter(true);
    m_collectionModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    m_orderProxy = new EntityOrderProxyModel(this);
    m_orderProxy->setSourceModel(m_collectionModel);
    KConfigGroup cfg(KSharedConfig::openConfig(), QStringLiteral("KJotsEntityOrder"));
    m_orderProxy->setOrderConfig(cfg);

    m_collectionView->setModel(m_orderProxy);

    m_collectionSelectionProxyModel = new KSelectionProxyModel(m_collectionView->selectionModel(), this);
    m_collectionSelectionProxyModel->setSourceModel(m_kjotsModel);
    m_collectionSelectionProxyModel->setFilterBehavior(KSelectionProxyModel::ChildrenOfExactSelection);

    m_itemModel = new EntityMimeTypeFilterModel(this);
    m_itemModel->setSourceModel(m_collectionSelectionProxyModel);
    m_itemModel->addMimeTypeExclusionFilter(Collection::mimeType());
    m_itemModel->setHeaderGroup(EntityTreeModel::ItemListHeaders);

    m_itemSortModel = new NoteSortProxyModel(this);
    m_itemSortModel->setSourceModel(m_itemModel);
    m_itemSortModel->setSortRole(Qt::EditRole);

    m_itemView->setModel(m_itemSortModel);

    m_actionManager->setCollectionSelectionModel(m_collectionView->selectionModel());
    m_actionManager->setItemSelectionModel(m_itemView->selectionModel());

    connect(m_kjotsModel, &EntityTreeModel::modelAboutToBeReset, this, &KJotsWidget::saveState);
    connect(m_kjotsModel, &EntityTreeModel::modelReset, this, &KJotsWidget::restoreState);

    // TODO: handle dataChanged properly, i.e. if item was changed from outside
    connect(m_collectionView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &KJotsWidget::renderSelection);
    connect(m_collectionView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &KJotsWidget::updateMenu);
    connect(m_collectionView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &KJotsWidget::updateCaption);
    connect(m_collectionView->model(), &QAbstractItemModel::dataChanged, this, &KJotsWidget::renderSelection);
    connect(m_collectionView->model(), &QAbstractItemModel::dataChanged, this, &KJotsWidget::updateCaption);

    connect(m_itemView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &KJotsWidget::renderSelection);
    connect(m_itemView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &KJotsWidget::updateMenu);
    connect(m_itemView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &KJotsWidget::updateCaption);
    connect(m_itemView->model(), &QAbstractItemModel::dataChanged, this, &KJotsWidget::renderSelection);
    connect(m_itemView->model(), &QAbstractItemModel::dataChanged, this, &KJotsWidget::updateCaption);

    connect(m_editor, &KJotsEdit::documentModified, this, &KJotsWidget::updateCaption);

    QTimer::singleShot(0, this, &KJotsWidget::delayedInitialization);

    // Autosave timer
    m_autosaveTimer = new QTimer(this);
    updateConfiguration();
    connect(m_autosaveTimer, &QTimer::timeout, m_editor, &KJotsEdit::savePage);
    connect(m_collectionView->selectionModel(), &QItemSelectionModel::selectionChanged, m_autosaveTimer, qOverload<>(&QTimer::start));

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
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Splitter between (collection view) and (item view + editor)
    m_splitter1 = new QSplitter(this);
    m_splitter1->setObjectName(QStringLiteral("CollectionSplitter"));
    m_splitter1->setStretchFactor(1, 1);
    layout->addWidget(m_splitter1);

    // Collection view
    m_collectionView = new EntityTreeView(m_xmlGuiClient, m_splitter1);
    m_collectionView->setObjectName(QStringLiteral("CollectionView"));
    m_collectionView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_collectionView->setEditTriggers(QAbstractItemView::DoubleClicked);
    m_collectionView->setManualSortingActive(true);
    m_collectionView->header()->setDefaultAlignment(Qt::AlignCenter);

    // Splitter between item view and editor
    m_splitter2 = new QSplitter(m_splitter1);
    m_splitter2->setObjectName(QStringLiteral("EditorSplitter"));

    // Item view
    m_itemView = new EntityTreeView(m_xmlGuiClient, m_splitter2);
    m_itemView->setObjectName(QStringLiteral("ItemView"));
    m_itemView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_itemView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_itemView->setEditTriggers(QAbstractItemView::DoubleClicked);
    m_itemView->setRootIsDecorated(false);
    m_itemView->header()->setDefaultAlignment(Qt::AlignCenter);

    // Stacked widget containing editor & browser
    m_stackedWidget = new QStackedWidget(m_splitter2);

    // Editor
    m_editor = new KJotsEdit(m_stackedWidget, m_xmlGuiClient->actionCollection());
    m_editorWidget = new TextCustomEditor::RichTextEditorWidget(m_editor, m_stackedWidget);
    m_editor->setParent(m_editorWidget);
    m_stackedWidget->addWidget(m_editorWidget);
    connect(m_editor, &KJotsEdit::linkClicked, this, &KJotsWidget::openLink);
    // Browser
    m_browserWidget = new KJotsBrowserWidget(std::make_unique<KJotsBrowser>(m_xmlGuiClient->actionCollection()),
                                             m_stackedWidget);
    m_stackedWidget->addWidget(m_browserWidget);
    m_stackedWidget->setCurrentWidget(m_browserWidget);
    connect(m_browserWidget->browser(), &KJotsBrowser::linkClicked, this, &KJotsWidget::openLink);

    // Make sure the editor gets focus again after naming a new book/page.
    connect(m_collectionView->itemDelegate(), &QItemDelegate::closeEditor, this, [this](){
            activeEditor()->setFocus();
        });
}

void KJotsWidget::restoreState()
{
    {
        auto saver = new ETMViewStateSaver;
        saver->setView(m_collectionView);
        KConfigGroup cfg(KSharedConfig::openConfig(), QStringLiteral("CollectionViewState"));
        saver->restoreState(cfg);
    }
    {
        auto saver = new ETMViewStateSaver;
        saver->setView(m_itemView);
        KConfigGroup cfg(KSharedConfig::openConfig(), QStringLiteral("ItemViewState"));
        saver->restoreState(cfg);
    }
}

void KJotsWidget::saveState()
{
    {
        ETMViewStateSaver saver;
        saver.setView(m_collectionView);
        KConfigGroup cfg(KSharedConfig::openConfig(), QStringLiteral("CollectionViewState"));
        saver.saveState(cfg);
        cfg.sync();
    }
    {
        ETMViewStateSaver saver;
        saver.setView(m_itemView);
        KConfigGroup cfg(KSharedConfig::openConfig(), QStringLiteral("ItemViewState"));
        saver.saveState(cfg);
        cfg.sync();
    }
}

void KJotsWidget::saveUIStates() const
{
    const QString groupName = QStringLiteral("UiState_MainWidget_%1").arg(KJotsSettings::viewMode());
    KConfigGroup group(KSharedConfig::openConfig(), groupName);
    KJots::UiStateSaver::saveState(m_splitter1, group);
    KJots::UiStateSaver::saveState(m_splitter2, group);
    KJots::UiStateSaver::saveState(m_collectionView, group);
    KJots::UiStateSaver::saveState(m_itemView, group);
    group.sync();
}

void KJotsWidget::restoreUIStates()
{
    const QString groupName = QStringLiteral("UiState_MainWidget_%1").arg(KJotsSettings::viewMode());
    KConfigGroup group(KSharedConfig::openConfig(), groupName);
    KJots::UiStateSaver::restoreState(m_splitter1, group);
    KJots::UiStateSaver::restoreState(m_splitter2, group);
    KJots::UiStateSaver::restoreState(m_collectionView, group);
    KJots::UiStateSaver::restoreState(m_itemView, group);
    group.sync();
}

void KJotsWidget::setViewMode(int mode)
{
    const int newMode = (mode == 0) ? KJotsSettings::viewMode() : mode;
    m_splitter2->setOrientation(newMode == 1 ? Qt::Vertical : Qt::Horizontal);
    if (mode != 0) {
        KJotsSettings::setViewMode(mode);
        saveUIStates();
    }
    restoreUIStates();
    m_viewModeGroup->actions().at(newMode-1)->setChecked(true);
}

void KJotsWidget::setupActions()
{
    KActionCollection *actionCollection = m_xmlGuiClient->actionCollection();
    // Default Akonadi Notes actions
    m_actionManager = new StandardNoteActionManager(actionCollection, this);
    m_actionManager->createAllActions();

    actionCollection->setDefaultShortcut(m_actionManager->action(StandardActionManager::CreateCollection),
                                         QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N));

    QAction *action;
    // Standard actions
    KStandardAction::preferences(this, &KJotsWidget::configure, actionCollection);

    action = KStandardAction::next(this, [this](){
            m_itemView->selectionModel()->select(previousNextEntity(m_itemView, +1), QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
        }, actionCollection);
    actionCollection->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::Key_PageDown));
    connect(this, &KJotsWidget::canGoNextPageChanged, action, &QAction::setEnabled);

    action = KStandardAction::prior(this, [this](){
            m_itemView->selectionModel()->select(previousNextEntity(m_itemView, -1), QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
        }, actionCollection);
    actionCollection->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::Key_PageUp));
    connect(this, &KJotsWidget::canGoPreviousPageChanged, action, &QAction::setEnabled);

    KStandardAction::renameFile(this, [this](){
            EntityTreeView *activeView;
            if (m_collectionView->hasFocus()) {
                activeView = m_collectionView;
            } else {
                activeView = m_itemView;
            }

            const QModelIndexList rows = activeView->selectionModel()->selectedRows();
            if (rows.size() != 1) {
                return;
            }
            activeView->edit(rows.first());
        }, actionCollection);

    action = KStandardAction::deleteFile(this, [this](){
            if (m_collectionView->hasFocus()) {
                m_actionManager->action(StandardActionManager::DeleteCollections)->trigger();
            } else {
                m_actionManager->action(StandardActionManager::DeleteItems)->trigger();
            }
        }, actionCollection);
    // Default Shift+Delete is ambiguous and used both for cut and delete
    actionCollection->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::Key_Delete));

    action = KStandardAction::copy(this, [this](){
            activeEditor()->copy();
        }, actionCollection);
    connect(m_editor, &KJotsEdit::copyAvailable, action, &QAction::setEnabled);
    connect(m_browserWidget->browser(), &KJotsBrowser::copyAvailable, action, &QAction::setEnabled);
    action->setEnabled(false);

    action = KStandardAction::selectAll(this, [this](){
            activeEditor()->selectAll();
        }, actionCollection);
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
    action = KStandardAction::replace(m_editorWidget, &TextCustomEditor::RichTextEditorWidget::slotReplace, actionCollection);
    connect(m_stackedWidget, &QStackedWidget::currentChanged, this, [this, action](int index){
            action->setEnabled(m_stackedWidget->widget(index) == m_editorWidget);
        });

    KStandardAction::print(this, &KJotsWidget::printSelection, actionCollection);
    KStandardAction::printPreview(this, &KJotsWidget::printPreviewSelection, actionCollection);

    // Bookmarks actions
    auto *bookmarkMenu = actionCollection->add<KActionMenu>(QStringLiteral("bookmarks"));
    bookmarkMenu->setText(i18n("&Bookmarks"));
    auto bookmarks = new KJotsBookmarks(m_collectionView->selectionModel(), this);
    connect(bookmarks, &KJotsBookmarks::openLink, this, &KJotsWidget::openLink);
    m_bookmarkManager = new KBookmarkManager(
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
        QStringLiteral("/kjots/bookmarks.xml"), this);
    auto bmm = new KBookmarkMenu(m_bookmarkManager, bookmarks,
                                 bookmarkMenu->menu());

    // "Add bookmark" and "make text bold" actions have conflicting shortcuts
    // (ctrl + b) Make add_bookmark use ctrl+shift+b to resolve that.
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

    // Move actions
    action = actionCollection->addAction(QStringLiteral("go_next_book"));
    action->setText(i18n("Next Book"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("go-down")));
    actionCollection->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_PageDown));
    connect(action, &QAction::triggered, this, [this](){
            const QModelIndex idx = previousNextEntity(m_collectionView, +1);
            m_collectionView->selectionModel()->select(idx, QItemSelectionModel::SelectCurrent);
            m_collectionView->expand(idx);
        });
    connect(this, &KJotsWidget::canGoNextBookChanged, action, &QAction::setEnabled);

    action = actionCollection->addAction(QStringLiteral("go_prev_book"));
    action->setText(i18n("Previous Book"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("go-up")));
    actionCollection->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_PageUp));
    connect(action, &QAction::triggered, this, [this](){
            const QModelIndex idx = previousNextEntity(m_collectionView, -1);
            m_collectionView->selectionModel()->select(idx, QItemSelectionModel::SelectCurrent);
            m_collectionView->expand(idx);
        });
    connect(this, &KJotsWidget::canGoPreviousBookChanged, action, &QAction::setEnabled);

    // View mode actions
    m_viewModeGroup = new QActionGroup(this);

    action = new QAction(i18nc("@action:inmenu", "Two Columns"), m_viewModeGroup);
    action->setCheckable(true);
    action->setData(1);
    actionCollection->addAction(QStringLiteral("view_mode_two_columns"), action);

    action = new QAction(i18nc("@action:inmenu", "Three Columns"), m_viewModeGroup);
    action->setCheckable(true);
    action->setData(2);
    actionCollection->addAction(QStringLiteral("view_mode_three_columns"), action);

    connect(m_viewModeGroup, &QActionGroup::triggered, this, [this](QAction *action){
            setViewMode(action->data().toInt());
        });
}

void KJotsWidget::delayedInitialization()
{
    // anySelectionActions are available when at least something is selected (i.e. editor/browser are not empty

    KActionCollection *actionCollection = m_xmlGuiClient->actionCollection();

    // Actions for a single item selection.
    anySelectionActions = { actionCollection->action(KStandardAction::name(KStandardAction::Find)),
                            actionCollection->action(KStandardAction::name(KStandardAction::Print)),
                            actionCollection->action(QStringLiteral("save_to")) };

    updateMenu();

    // Load view mode and splitters
    setViewMode(0);
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
    const int collectionsSelected = m_collectionView->selectionModel()->selectedRows().count();
    const int itemsSelected = m_itemView->selectionModel()->selectedRows().count();
    const int selectionSize = itemsSelected + collectionsSelected;

    // Actions available only when editor is shown
    m_editor->setEnableActions(itemsSelected == 1 && !m_editor->locked());

    // Rename is available only when single something is selected
    m_xmlGuiClient->actionCollection()
            ->action(KStandardAction::name(KStandardAction::RenameFile))
            ->setEnabled((itemsSelected == 1) || (m_collectionView->hasFocus() && collectionsSelected == 1));

    // Actions available when at least something is shown
    for (QAction *action : std::as_const(anySelectionActions)) {
        action->setEnabled(selectionSize >= 1);
    }
}

void KJotsWidget::configure()
{
    if (KConfigDialog::showDialog(QStringLiteral("kjotssettings"))) {
        return;
    }
    auto* dialog = new KConfigDialog(this, QStringLiteral("kjotssettings"), KJotsSettings::self());
    auto configMisc = new KJotsConfigMisc(dialog);
    dialog->addPage(configMisc->widget(), i18nc("@title:window config dialog page", "Misc"), QStringLiteral("preferences-other"));
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

    const QModelIndexList selectedItems = m_itemView->selectionModel()->selectedRows();
    if (selectedItems.count() > 0) {
        objectList.reserve(selectedItems.size());
        std::transform(selectedItems.cbegin(), selectedItems.cend(), std::back_inserter(objectList),
            [](const QModelIndex &idx){
                return idx.data(KJotsModel::GrantleeObjectRole);
            });
    } else {
        const QModelIndexList selectedCollections = m_collectionView->selectionModel()->selectedRows();
        objectList.reserve(selectedCollections.size());
        std::transform(selectedCollections.cbegin(), selectedCollections.cend(), std::back_inserter(objectList),
            [](const QModelIndex &idx){
                return idx.data(KJotsModel::GrantleeObjectRole);
            });
    }
    QHash<QString, QVariant> hash = {{QStringLiteral("entities"), objectList},
                                     {QStringLiteral("i18n_TABLE_OF_CONTENTS"), i18nc("Header for 'Table of contents' section of rendered output", "Table of contents")}};
    KTextTemplate::Context c(hash);

    const QString currentTheme = m_loader->themeName();
    m_loader->setTheme(theme);
    KTextTemplate::Template t = m_templateEngine->loadByName(templ);
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

QModelIndex KJotsWidget::previousNextEntity(QTreeView *view, int step)
{
    const QModelIndexList selection = view->selectionModel()->selectedRows();
    if (selection.size() == 0) {
        return step > 0 ? view->model()->index(0, 0) : view->model()->index(view->model()->rowCount()-1, 0);
    }
    if (selection.size() != 1) {
        return {};
    }
    return step > 0 ? view->indexBelow(selection.first()) : view->indexAbove(selection.first());
}

void KJotsWidget::renderSelection()
{
    Q_EMIT canGoNextBookChanged(previousNextEntity(m_collectionView, +1).isValid());
    Q_EMIT canGoNextPageChanged(previousNextEntity(m_itemView, +1).isValid());
    Q_EMIT canGoPreviousBookChanged(previousNextEntity(m_collectionView, -1).isValid());
    Q_EMIT canGoPreviousPageChanged(previousNextEntity(m_itemView, -1).isValid());

    const QModelIndexList selectedItems = m_itemView->selectionModel()->selectedRows();

    // If the selection is a single note, present it for editing...
    if (selectedItems.count() == 1) {
        if (m_editor->setModelIndex(selectedItems.first())) {
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
    QString caption;
    const QModelIndexList itemSelection = m_itemView->selectionModel()->selectedRows();
    const QModelIndexList collectionSelection = m_collectionView->selectionModel()->selectedRows();
    if (itemSelection.size() == 1) {
        caption = KJotsModel::itemPath(KJotsModel::etmIndex(itemSelection.first()));
        if (m_editor->modified()) {
            caption.append(QStringLiteral(" *"));
        }
    } else if (itemSelection.size() == 0 && collectionSelection.size() == 1) {
        caption = KJotsModel::itemPath(collectionSelection.first());
    } else if (itemSelection.size() > 1 || collectionSelection.size() > 1) {
        caption = i18nc("@title:window", "Multiple selection");
    }

    Q_EMIT captionChanged(caption);
}

bool KJotsWidget::queryClose()
{
    // Saving the current note
    // We cannot use async interface (i.e. ETM) here
    // because we need to abort the close if something went wrong
    const QModelIndexList selection = m_itemView->selectionModel()->selectedRows();
    if ((selection.size() == 1) && (m_editor->document()->isModified())) {
        QModelIndex idx = selection.first();
        m_editor->prepareDocumentForSaving();
        auto job = new ItemModifyJob(KJotsModel::updateItem(idx.data(EntityTreeModel::ItemRole).value<Item>(), m_editor->document()));
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
        }
    }

    saveUIStates();
    KJotsSettings::self()->save();
    m_orderProxy->saveOrder();

    return true;
}

void KJotsWidget::openLink(const QUrl &url)
{
    if (url.scheme() == QStringLiteral("akonadi")) {
        QModelIndex idx = KJotsModel::modelIndexForUrl(m_kjotsModel, url);

        // Trying to map it to collection view model
        QModelIndex colIdx = m_collectionModel->mapFromSource(idx);
        if (colIdx.isValid()) {
            colIdx = m_orderProxy->mapFromSource(colIdx);
            m_collectionView->selectionModel()->select(colIdx, QItemSelectionModel::SelectCurrent);
            m_itemView->selectionModel()->clearSelection();
        } else {
            // Selecting parent collection
            QModelIndex parentCollectionIdx = EntityTreeModel::modelIndexForCollection(m_collectionView->model(),
                                                                                       idx.data(EntityTreeModel::ParentCollectionRole).value<Collection>());
            m_collectionView->selectionModel()->select(parentCollectionIdx, QItemSelectionModel::SelectCurrent);

            // Mapping idx to item view model
            idx = m_collectionSelectionProxyModel->mapFromSource(idx);
            idx = m_itemModel->mapFromSource(idx);
            idx = m_itemSortModel->mapFromSource(idx);
            m_itemView->selectionModel()->select(idx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
        }
    } else {
        auto job = new KIO::OpenUrlJob(url, this);
        job->start();
    }
}

#include "moc_kjotswidget.cpp"
