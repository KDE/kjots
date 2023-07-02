/*
    This file is part of KJots.

    SPDX-FileCopyrightText: 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
                  2002, 2003 Aaron J. Seigo <aseigo@kde.org>
                  2003 Stanislav Kljuhhin <crz@hot.ee>
                  2005-2006 Jaison Lee <lee.jaison@gmail.com>
                  2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

//Own Header
#include "kjotsbrowser.h"
#include "kjotsmodel.h"

#ifdef HAVE_TEXT_TO_SPEECH_SUPPORT
#include <TextEditTextToSpeech/TextToSpeechWidget>
#endif

#include <KPIMTextEdit/RichTextEditFindBar>
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
#ifdef HAVE_TEXT_TO_SPEECH_SUPPORT
        , mTextToSpeechWidget(widget)
#endif
    {
    }

    std::unique_ptr<KJotsBrowser> mBrowser;
    KPIMTextEdit::SlideContainer mSliderContainer;
    KPIMTextEdit::RichTextEditFindBar mFindBar;
#ifdef HAVE_TEXT_TO_SPEECH_SUPPORT
    TextEditTextToSpeech::TextToSpeechWidget mTextToSpeechWidget;
#endif
};

KJotsBrowserWidget::KJotsBrowserWidget(std::unique_ptr<KJotsBrowser> browser, QWidget *parent)
    : QWidget(parent)
    , d(new KJotsBrowserWidgetPrivate(std::move(browser), this))
{
    d->mBrowser->setParent(this);
    d->mSliderContainer.setContent(&d->mFindBar);
    d->mFindBar.setHideWhenClose(false);

    connect(&d->mFindBar, &KPIMTextEdit::RichTextEditFindBar::hideFindBar, this, &KJotsBrowserWidget::slotHideFindBar);
#ifdef HAVE_TEXT_TO_SPEECH_SUPPORT
    connect(d->mBrowser.get(), &KJotsBrowser::say, &d->mTextToSpeechWidget, &TextEditTextToSpeech::TextToSpeechWidget::say);
#endif

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
#ifdef HAVE_TEXT_TO_SPEECH_SUPPORT
    lay->addWidget(&d->mTextToSpeechWidget);
#endif
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
#ifdef HAVE_TEXT_TO_SPEECH_SUPPORT
    popup->addSeparator();
    if (!document()->isEmpty() && TextEditTextToSpeech::TextToSpeech::self()->isReady()) {
        QAction *speakAction = popup->addAction(i18nc("@info:action", "Speak Text"));
        speakAction->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-text-to-speech")));
        connect(speakAction, &QAction::triggered, this, [this](){
                const QString text = textCursor().hasSelection() ? textCursor().selectedText() : document()->toPlainText();
                Q_EMIT say(text);
            });
    }
#endif
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            Item::Id id = url.fragment().midRef(5).toInt(&ok);
#else
            Item::Id id = QStringView(url.fragment()).mid(5).toInt(&ok);
#endif
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

#include "moc_kjotsbrowser.cpp"
