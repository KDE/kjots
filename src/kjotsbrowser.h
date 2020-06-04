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
