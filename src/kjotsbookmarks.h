/*
    This file is part of KJots.

    SPDX-FileCopyrightText: 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
                  2002, 2003 Aaron J. Seigo <aseigo@kde.org>
                  2003 Stanislav Kljuhhin <crz@hot.ee>
                  2005-2006 Jaison Lee <lee.jaison@gmail.com>
                  2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KJOTSBOOKMARKS
#define KJOTSBOOKMARKS

#include <KBookmarkManager>
#include <KBookmarkOwner>

class QItemSelectionModel;

class KJotsBookmarks : public QObject, public KBookmarkOwner
{
    Q_OBJECT

public:
    explicit KJotsBookmarks(QItemSelectionModel *model, QObject *parent = nullptr);

    QUrl currentUrl() const override;
    QString currentIcon() const override;
    QString currentTitle() const override;
    void openBookmark(const KBookmark &bm, Qt::MouseButtons mb, Qt::KeyboardModifiers km) override;

Q_SIGNALS:
    void openLink(const QUrl &url);

private:
    QItemSelectionModel *m_model = nullptr;
};

#endif
/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
