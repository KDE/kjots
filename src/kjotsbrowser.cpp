/*
    kjotsbookmarks

    Copyright (C) 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
    Copyright (C) 2002, 2003 Aaron J. Seigo <aseigo@kde.org>
    Copyright (C) 2003 Stanislav Kljuhhin <crz@hot.ee>
    Copyright (C) 2005-2006 Jaison Lee <lee.jaison@gmail.com>

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
#include "kjotsbrowser.h"
#include "kjotsmodel.h"

#include <KPIMTextEdit/RichTextEditFindBar>
#include <KPIMTextEdit/TextToSpeechWidget>
#include <KPIMTextEdit/SlideContainer>

#include <QHelpEvent>
#include <QToolTip>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>

#include <KActionCollection>
#include <KLocalizedString>
#include <KStandardAction>

using namespace Akonadi;

class Q_DECL_HIDDEN KJotsBrowserWidgetPrivate
{
public:
    explicit KJotsBrowserWidgetPrivate(std::unique_ptr<KJotsBrowser> browser, QWidget *widget)
        : mBrowser(std::move(browser))
        , mSliderContainer(widget)
        , mFindBar(mBrowser.get(), &mSliderContainer)
        , mTextToSpeechWidget(widget)
    {
    }

    std::unique_ptr<KJotsBrowser> mBrowser;
    KPIMTextEdit::SlideContainer mSliderContainer;
    KPIMTextEdit::RichTextEditFindBar mFindBar;
    KPIMTextEdit::TextToSpeechWidget mTextToSpeechWidget;
};

KJotsBrowserWidget::KJotsBrowserWidget(std::unique_ptr<KJotsBrowser> browser, QWidget *parent)
    : QWidget(parent)
    , d(new KJotsBrowserWidgetPrivate(std::move(browser), this))
{
    d->mBrowser->setParent(this);
    d->mSliderContainer.setContent(&d->mFindBar);
    d->mFindBar.setHideWhenClose(false);

    connect(&d->mFindBar, &KPIMTextEdit::RichTextEditFindBar::hideFindBar, this, &KJotsBrowserWidget::slotHideFindBar);
    connect(d->mBrowser.get(), &KJotsBrowser::say, &d->mTextToSpeechWidget, &KPIMTextEdit::TextToSpeechWidget::say);

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(&d->mTextToSpeechWidget);
    lay->addWidget(d->mBrowser.get());
    lay->addWidget(&d->mSliderContainer);
}

KJotsBrowserWidget::~KJotsBrowserWidget() = default;

KJotsBrowser* KJotsBrowserWidget::browser()
{
    return d->mBrowser.get();
}

void KJotsBrowserWidget::slotFind()
{
    if (d->mBrowser->textCursor().hasSelection()) {
        d->mFindBar.setText(d->mBrowser->textCursor().selectedText());
    }
    d->mBrowser->moveCursor(QTextCursor::Start);

    d->mFindBar.showFind();
    d->mSliderContainer.slideIn();
    d->mFindBar.focusAndSetCursor();
}

void KJotsBrowserWidget::slotFindNext()
{
    if (d->mFindBar.isVisible()) {
        d->mFindBar.findNext();
    } else {
        slotFind();
    }
}

void KJotsBrowserWidget::slotHideFindBar()
{
    d->mSliderContainer.slideOut();
    d->mBrowser->setFocus();
}

KJotsBrowser::KJotsBrowser(KActionCollection *actionCollection, QWidget *parent)
    : QTextBrowser(parent)
    , m_actionCollection(actionCollection)
{
    setWordWrapMode(QTextOption::WordWrap);

    connect(this, &KJotsBrowser::anchorClicked, this, [this](const QUrl &url){
        if (!url.toString().startsWith(QLatin1Char('#'))) {
            // QTextBrowser tries to automatically handle the url. We only want it for anchor navigation
            // (i.e. "#page12" links). This can be overridden by setting the source to an invalid QUrl
            setSource(QUrl());
            Q_EMIT linkClicked(url);
        }
    });
}

void KJotsBrowser::setModel(QAbstractItemModel *model)
{
    m_model = model;
}

void KJotsBrowser::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *popup = createStandardContextMenu(event->pos());
    if (!popup) {
        return;
    }
    popup->addSeparator();
    popup->addAction(m_actionCollection->action(QString::fromLatin1(KStandardAction::name(KStandardAction::Find))));
    popup->addSeparator();
    if (!document()->isEmpty() && KPIMTextEdit::TextToSpeech::self()->isReady()) {
        QAction *speakAction = popup->addAction(i18nc("@info:action", "Speak Text"));
        speakAction->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-text-to-speech")));
        connect(speakAction, &QAction::triggered, this, [this](){
                const QString text = textCursor().hasSelection() ? textCursor().selectedText() : document()->toPlainText();
                Q_EMIT say(text);
            });
    }
    popup->exec(event->globalPos());
    delete popup;
}

bool KJotsBrowser::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        tooltipEvent(static_cast<QHelpEvent*>(event));
    }
    return QTextBrowser::event(event);
}

void KJotsBrowser::tooltipEvent(QHelpEvent *event)
{
    if (!m_model) {
        return;
    }
    // This code is somewhat shared with KJotsEdit
    QUrl url(anchorAt(event->pos()));
    QString message;

    if (url.isValid()) {
        QModelIndex idx;
        if (url.scheme() == QStringLiteral("akonadi")) {
            idx = KJotsModel::modelIndexForUrl(m_model, url);
        } else
        // This is #page_XXX internal links
        if (url.scheme().isEmpty() && url.host().isEmpty() && url.path().isEmpty() && url.query().isEmpty()
                   && url.fragment().startsWith(QLatin1String("page_")))
        {
            bool ok;
            Item::Id id = url.fragment().midRef(5).toInt(&ok);
            const QModelIndexList idxs = EntityTreeModel::modelIndexesForItem(m_model, Item(id));
            if (ok && !idxs.isEmpty()) {
                idx = idxs.first();
            }
        } else {
            message = i18nc("@info:tooltip %1 is hyperlink address", "Click to follow the hyperlink: %1", url.toString(QUrl::RemovePassword));
        }
        if (idx.isValid()) {
            if (idx.data(EntityTreeModel::ItemRole).value<Item>().isValid()) {
                message = i18nc("@info:tooltip %1 is a full path to note (i.e. Notes / Notebook / Note)", "Click to open note: %1", KJotsModel::itemPath(idx));
            } else if (idx.data(EntityTreeModel::CollectionRole).value<Collection>().isValid()) {
                message = i18nc("@info:tooltip %1 is a full path to book (i.e. Notes / Notebook)", "Click to open book: %1", KJotsModel::itemPath(idx));
            }
        }
    }

    if (!message.isEmpty()) {
        QToolTip::showText(event->globalPos(), message);
    } else {
        QToolTip::hideText();
    }
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
