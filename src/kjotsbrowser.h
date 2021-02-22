/*
    This file is part of KJots.

    SPDX-FileCopyrightText: 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
                  2002, 2003 Aaron J. Seigo <aseigo@kde.org>
                  2003 Stanislav Kljuhhin <crz@hot.ee>
                  2005-2006 Jaison Lee <lee.jaison@gmail.com>
                  2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KJOTSBROWSER_H
#define KJOTSBROWSER_H

#include <QTextBrowser>

class QHelpEvent;
class QAbstractItemModel;

class KActionCollection;

class KJotsBrowserWidgetPrivate;

class KJotsBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    explicit KJotsBrowser(KActionCollection *actionCollection, QWidget *parent = nullptr);
    /**
     * @brief set the ETM which will be used to display
     * additional information in tooltips
     */
    void setModel(QAbstractItemModel *model);
protected:
    bool event(QEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void tooltipEvent(QHelpEvent *event);
Q_SIGNALS:
    void linkClicked(const QUrl &);
    void say(const QString &text);
private:
    QAbstractItemModel *m_model = nullptr;
    KActionCollection *m_actionCollection = nullptr;
};

/**
 * @brief A widget-wrapper around KJotsBrowser
 *
 * It also contains: find bar, text-to-speech widget.
 * @see KPIMTextEdit::RichTextEditorWidget
 */
class KJotsBrowserWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KJotsBrowserWidget(std::unique_ptr<KJotsBrowser> browser, QWidget *parent = nullptr);
    ~KJotsBrowserWidget();

    KJotsBrowser *browser();
public Q_SLOTS:
    void slotFind();
    void slotFindNext();
private:
    void slotHideFindBar();
    std::unique_ptr<KJotsBrowserWidgetPrivate> const d;
};


#endif
/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
