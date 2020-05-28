/*
    kjots

    Copyright (C) 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
    Copyright (C) 2002, 2003 Aaron J. Seigo <aseigo@kde.org>
    Copyright (C) 2003 Stanislav Kljuhhin <crz@hot.ee>
    Copyright (C) 2005-2006 Jaison Lee <lee.jaison@gmail.com>
    Copyright (C) 2007-2008 Stephen Kelly <steveire@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

//Own Header
#include "kjotsedit.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QMimeData>
#include <QTextCursor>
#include <QTextDocumentFragment>
#include <QToolTip>
#include <QUrl>

#include <KActionCollection>
#include <KStandardGuiItem>
#include <KLocalizedString>
#include <KMime/Message>
#include <KPIMTextEdit/RichTextComposerControler>
#include <KPIMTextEdit/RichTextComposerActions>

#include <AkonadiCore/Item>

#include "kjotslinkdialog.h"
#include "kjotsmodel.h"
#include "noteshared/noteeditorutils.h"
#include "noteshared/notelockattribute.h"

Q_DECLARE_METATYPE(QTextCursor)

using namespace Akonadi;
using namespace KPIMTextEdit;

class Q_DECL_HIDDEN KJotsEdit::Private {
public:
    Private() = default;
    ~Private() = default;

    QAction *action_copy_into_title = nullptr;
    QAction *action_manage_link = nullptr;
    QAction *action_auto_bullet = nullptr;
    QAction *action_auto_decimal = nullptr;
    QAction *action_insert_date = nullptr;
    QVector<QAction *> richTextActionList;
};

KJotsEdit::KJotsEdit(QWidget *parent, KActionCollection *actionCollection)
    : RichTextComposer(parent)
    , d(new Private)
    , m_actionCollection(actionCollection)
    , allowAutoDecimal(false)
{
    setMouseTracking(true);
    setAcceptRichText(true);
    setWordWrapMode(QTextOption::WordWrap);
    setCheckSpellingEnabled(true);
    setFocusPolicy(Qt::StrongFocus);

    createActions(m_actionCollection);
    activateRichText();
}

KJotsEdit::~KJotsEdit() = default;

void KJotsEdit::createActions(KActionCollection *ac)
{
    RichTextComposer::createActions(ac);

    d->action_copy_into_title = new QAction(QIcon::fromTheme(QStringLiteral("edit-copy")),
                                            i18nc("@action", "Copy &Into Page Title"), this);
    connect(d->action_copy_into_title, &QAction::triggered, this, &KJotsEdit::copySelectionIntoTitle);
    connect(this, &KJotsEdit::copyAvailable, d->action_copy_into_title, &QAction::setEnabled);
    d->action_copy_into_title->setEnabled(false);
    d->richTextActionList.append(d->action_copy_into_title);
    if (ac) {
        ac->addAction(QStringLiteral("copy_into_title"), d->action_copy_into_title);
    }

    d->action_manage_link = new QAction(QIcon::fromTheme(QStringLiteral("insert-link")),
                                        i18nc("@action creates and manages hyperlinks", "Link"), this);
    connect(d->action_manage_link, &QAction::triggered, this, &KJotsEdit::onLinkify);
    d->richTextActionList.append(d->action_manage_link);
    if (ac) {
        ac->addAction(QStringLiteral("manage_note_link"), d->action_manage_link);
    }

    d->action_auto_bullet = new QAction(QIcon::fromTheme(QStringLiteral("format-list-unordered")),
                                        i18nc("@action", "Auto Bullet List"), this);
    d->action_auto_bullet->setCheckable(true);
    connect(d->action_auto_bullet, &QAction::triggered, this, &KJotsEdit::onAutoBullet);
    d->richTextActionList.append(d->action_auto_bullet);
    if (ac) {
        ac->addAction(QStringLiteral("auto_bullet"), d->action_auto_bullet);
    }

    d->action_auto_decimal = new QAction(QIcon::fromTheme(QStringLiteral("format-list-ordered")),
                                         i18nc("@action", "Auto Decimal List"), this);
    d->action_auto_decimal->setCheckable(true);
    connect(d->action_auto_decimal, &QAction::triggered, this, &KJotsEdit::onAutoDecimal);
    d->richTextActionList.append(d->action_auto_decimal);
    if (ac) {
        ac->addAction(QStringLiteral("auto_decimal"), d->action_auto_decimal);
    }

    d->action_insert_date = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-time-spent")),
                                        i18nc("@action", "Insert Date"), this);
    connect(d->action_insert_date, &QAction::triggered, this, &KJotsEdit::insertDate);
    d->richTextActionList.append(d->action_insert_date);
    if (ac) {
        ac->addAction(QStringLiteral("insert_date"), d->action_insert_date);
        ac->setDefaultShortcut(d->action_insert_date, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_I));
    }
}

void KJotsEdit::setEnableActions(bool enable)
{
    // FIXME: RichTextComposer::setEnableActions(enable) messes with indent actions
    // due to bug in KPIMTextEdit (should be fixed in 20.08?)
    composerActions()->setActionsEnabled(enable);
    for (QAction *action : qAsConst(d->richTextActionList)) {
        action->setEnabled(enable);
    }
}

void KJotsEdit::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *popup = mousePopupMenu(event->pos());
    if (popup) {
        const QList<QAction*> actionList = popup->actions();
        if (!qApp->clipboard()->text().isEmpty()) {
            QAction *act = m_actionCollection->action(QStringLiteral("paste_without_formatting"));
            act->setIcon(QIcon::fromTheme(QStringLiteral("edit-paste")));
            act->setEnabled(!isReadOnly());
            // HACK: menu actions are following: Undo, Redo, Separator, Cut, Copy, Paste, Delete, Clear
            // We want to insert "Paste Without Formatting" right after standard Paste (which is at pos 6)
            // Let's hope QTextEdit and KPIMTextEdit::RichTextEditor doesn't break it
            // (and we don't break anything either)
            const int pasteActionPosition = 6;
            if (actionList.count() >= pasteActionPosition) {
                popup->insertAction(popup->actions().at(pasteActionPosition), act);
            } else {
                popup->addAction(act);
            }
        }
        popup->addSeparator();
        popup->addAction(d->action_copy_into_title);

        if (!anchorAt(event->pos()).isNull()) {
            popup->addAction(d->action_manage_link);
        }

        popup->exec(event->globalPos());
        delete popup;
    }
}

bool KJotsEdit::modified()
{
    return document()->isModified();
}

void KJotsEdit::insertDate()
{
    NoteShared::NoteEditorUtils::insertDate(this);
}

bool KJotsEdit::setModelIndex(const QModelIndex &index)
{
    bool newDocument = m_index && (*m_index != index);
    // Saving the old document, if it wa changed
    if (newDocument) {
        savePage();
    }
    m_index = std::make_unique<QPersistentModelIndex>(index);
    // Loading document
    auto *doc = m_index->data(KJotsModel::DocumentRole).value<QTextDocument *>();
    if (!doc) {
        setReadOnly(true);
        return false;
    }

    disconnect(document(), &QTextDocument::modificationChanged, this, &KJotsEdit::documentModified);
    setDocument(doc);
    connect(doc, &QTextDocument::modificationChanged, this, &KJotsEdit::documentModified);

    // Setting cursor
    auto cursor = doc->property("textCursor").value<QTextCursor>();
    if (!cursor.isNull()) {
        setTextCursor(cursor);
    } else {
        // This is a work-around for QTextEdit bug. If the first letter of the document is formatted,
        // QTextCursor doesn't follow this format. One can either move the cursor 1 symbol to the right
        // and then 1 symbol to the left as a workaround, or just explicitly move it to the start.
        // Submitted to qt-bugs, id 192886.
        //  -- (don't know the fate of this bug, as for April 2020 it is inaccessible)
        moveCursor(QTextCursor::Start);
    }
    // Setting focus if document was changed
    if (newDocument) {
        setFocus();
    }
    // Setting ReadOnly
    auto item = m_index->data(EntityTreeModel::ItemRole).value<Item>();
    if (!item.isValid()) {
        setReadOnly(true);
        return false;
    } else if (item.hasAttribute<NoteShared::NoteLockAttribute>()) {
        setReadOnly(true);
        return true;
    } else {
        setReadOnly(false);
        return true;
    }
}

void KJotsEdit::onAutoBullet()
{
    KTextEdit::AutoFormatting currentFormatting = autoFormatting();

    //TODO: set line spacing properly.

    if (currentFormatting == KTextEdit::AutoBulletList) {
        setAutoFormatting(KTextEdit::AutoNone);
        d->action_auto_bullet->setChecked(false);
    } else {
        setAutoFormatting(KTextEdit::AutoBulletList);
        d->action_auto_bullet->setChecked(true);
    }
}

void KJotsEdit::createAutoDecimalList()
{
    //this is an adaptation of Qt's createAutoBulletList() function for creating a bulleted list, except in this case I use it to create a decimal list.
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();

    QTextBlockFormat blockFmt = cursor.blockFormat();

    QTextListFormat listFmt;
    listFmt.setStyle(QTextListFormat::ListDecimal);
    listFmt.setIndent(blockFmt.indent() + 1);

    blockFmt.setIndent(0);
    cursor.setBlockFormat(blockFmt);

    cursor.createList(listFmt);

    cursor.endEditBlock();
    setTextCursor(cursor);
}

void KJotsEdit::DecimalList()
{
    QTextCursor cursor = textCursor();

    if (cursor.currentList()) {
        return;
    }

    QString blockText = cursor.block().text();

    if (blockText.length() == 2 && blockText == QLatin1String("1.")) {
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        createAutoDecimalList();
    }
}

void KJotsEdit::onAutoDecimal()
{
    if (allowAutoDecimal) {
        allowAutoDecimal = false;
        disconnect(this, &KJotsEdit::textChanged, this, &KJotsEdit::DecimalList);
        d->action_auto_decimal->setChecked(false);
    } else {
        allowAutoDecimal = true;
        connect(this, &KJotsEdit::textChanged, this, &KJotsEdit::DecimalList);
        d->action_auto_decimal->setChecked(true);
    }
}

void KJotsEdit::onLinkify()
{
    // Nothing is yet opened, ignoring
    if (!m_index) {
        return;
    }
    composerControler()->selectLinkText();
    auto linkDialog = std::make_unique<KJotsLinkDialog>(const_cast<QAbstractItemModel*>(m_index->model()), this);
    linkDialog->setLinkText(composerControler()->currentLinkText());
    linkDialog->setLinkUrl(composerControler()->currentLinkUrl());

    if (linkDialog->exec()) {
        composerControler()->updateLink(linkDialog->linkUrl(), linkDialog->linkText());
    }
}

void KJotsEdit::copySelectionIntoTitle()
{
    if (!m_index) {
        return;
    }
    const QString newTitle(textCursor().selectedText());
    auto *model = const_cast<QAbstractItemModel *>(m_index->model());
    model->setData(*m_index, newTitle);
}

bool KJotsEdit::canInsertFromMimeData(const QMimeData *source) const
{
    if (source->hasUrls()) {
        return true;
    } else {
        return RichTextComposer::canInsertFromMimeData(source);
    }
}

void KJotsEdit::insertFromMimeData(const QMimeData *source)
{
    // Nothing is opened, ignoring
    if (!m_index) {
        return;
    }
    if (source->hasUrls()) {
        const QList<QUrl> urls = source->urls();
        for (const QUrl &url : urls) {
            if (url.scheme() == QStringLiteral("akonadi")) {
                QModelIndex idx = KJotsModel::modelIndexForUrl(m_index->model(), url);
                if (idx.isValid()) {
                    insertHtml(QStringLiteral("<a href=\"%1\">%2</a>").arg(idx.data(KJotsModel::EntityUrlRole).toString(),
                                                                           idx.data().toString()));
                }
            } else {
                QString text = source->hasText() ? source->text() : url.toString(QUrl::RemovePassword);
                insertHtml(QStringLiteral("<a href=\"%1\">%2</a>").arg(QString::fromUtf8(url.toEncoded()), text));
            }
        }
    } else if (source->hasHtml()) {
        // Don't have an action to set top and bottom margins on paragraphs yet.
        // Remove the margins for all inserted html.
        QTextDocument dummy;
        dummy.setHtml(source->html());
        QTextCursor c(&dummy);
        QTextBlockFormat fmt = c.blockFormat();
        fmt.setTopMargin(0);
        fmt.setBottomMargin(0);
        fmt.setLeftMargin(0);
        fmt.setRightMargin(0);
        c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        c.mergeBlockFormat(fmt);

        textCursor().insertFragment(QTextDocumentFragment(c));
        ensureCursorVisible();
    } else {
        RichTextComposer::insertFromMimeData(source);
    }
}

void KJotsEdit::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->modifiers() & Qt::ControlModifier) && !anchorAt(event->pos()).isEmpty()) {
        if (!m_cursorChanged) {
            QApplication::setOverrideCursor(Qt::PointingHandCursor);
            m_cursorChanged = true;
        }
    } else {
        if (m_cursorChanged) {
            QApplication::restoreOverrideCursor();
            m_cursorChanged = false;
        }
    }
    RichTextComposer::mouseMoveEvent(event);
}

void KJotsEdit::leaveEvent(QEvent *event)
{
    if (m_cursorChanged) {
        QApplication::restoreOverrideCursor();
        m_cursorChanged = false;
    }
    RichTextComposer::leaveEvent(event);
}

void KJotsEdit::mousePressEvent(QMouseEvent *event)
{
    QUrl url = anchorAt(event->pos());
    if ((event->modifiers() & Qt::ControlModifier) && (event->button() & Qt::LeftButton) && !url.isEmpty()) {
        Q_EMIT linkClicked(url);
    } else {
        RichTextComposer::mousePressEvent(event);
    }
}

bool KJotsEdit::event(QEvent *event)
{
    if (event->type() == QEvent::WindowDeactivate) {
        savePage();
    } else if (event->type() == QEvent::ToolTip) {
        tooltipEvent(static_cast<QHelpEvent *>(event));
    }
    return RichTextComposer::event(event);
}

void KJotsEdit::tooltipEvent(QHelpEvent *event)
{
    // Nothing is opened, ignoring
    if (!m_index) {
        return;
    }
    QUrl url(anchorAt(event->pos()));
    QString message;

    if (url.isValid()) {
        if (url.scheme() == QStringLiteral("akonadi")) {
            const QModelIndex idx = KJotsModel::modelIndexForUrl(m_index->model(), url);
            if (idx.data(EntityTreeModel::ItemRole).value<Item>().isValid()) {
                message = i18nc("@info:tooltip %1 is a full path to note (i.e. Notes / Notebook / Note)", "Ctrl+click to open note: %1", KJotsModel::itemPath(idx));
            } else if (idx.data(EntityTreeModel::CollectionRole).value<Collection>().isValid()) {
                message = i18nc("@info:tooltip %1 is a full path to book (i.e. Notes / Notebook)", "Ctrl+click to open book: %1", KJotsModel::itemPath(idx));
            }
        } else {
            message = i18nc("@info:tooltip %1 is hyperlink address", "Ctrl+click to follow the hyperlink: %1", url.toString(QUrl::RemovePassword));
        }
    }

    if (!message.isEmpty()) {
        QToolTip::showText(event->globalPos(), message);
    } else {
        QToolTip::hideText();
    }
}

void KJotsEdit::focusOutEvent(QFocusEvent *event)
{
    savePage();
    RichTextComposer::focusOutEvent(event);
}

void KJotsEdit::savePage()
{
    if (!document()->isModified() || !m_index) {
        return;
    }

    auto item = m_index->data(EntityTreeModel::ItemRole).value<Item>();
    if (!item.isValid() || !item.hasPayload<KMime::Message::Ptr>()) {
        return;
    }
    auto *model = const_cast<QAbstractItemModel *>(m_index->model());
    document()->setModified(false);
    document()->setProperty("textCursor", QVariant::fromValue(textCursor()));
    model->setData(*m_index, QVariant::fromValue(document()), KJotsModel::DocumentRole);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
