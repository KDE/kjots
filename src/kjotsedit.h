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


#ifndef KJOTSEDIT_H
#define KJOTSEDIT_H

#include <KRichTextWidget>

class QItemSelection;
class QItemSelectionModel;

class KActionCollection;

class KJotsEdit : public KRichTextWidget
{
    Q_OBJECT
public:
    explicit KJotsEdit(QWidget *);

    void delayedInitialization(KActionCollection *);
    bool canInsertFromMimeData(const QMimeData *) const override;
    void insertFromMimeData(const QMimeData *) override;
    /**
     * Load document based on KJotsModel index
     *
     * @returns true if loaded sucessfully
     */
    bool setModelIndex(const QModelIndex &index);
protected:
    /* To be able to change cursor when hoverng over links */
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

    /** Override to make ctrl+click follow links */
    void mousePressEvent(QMouseEvent *) override;

    void contextMenuEvent(QContextMenuEvent *event) override;

    void focusOutEvent(QFocusEvent *) override;

    bool event(QEvent *event) override;

    void tooltipEvent(QHelpEvent *event);

public Q_SLOTS:
    void onAutoBullet(void);
    void onLinkify(void);
    void addCheckmark(void);
    void onAutoDecimal(void);
    void DecimalList(void);
    void pastePlainText(void);

    void savePage();
    void insertDate();
    void insertImage();

Q_SIGNALS:
    void linkClicked(const QUrl &url);
private:
    void createAutoDecimalList();
    KActionCollection *actionCollection;
    bool allowAutoDecimal;
    std::unique_ptr<QPersistentModelIndex> m_index;

    bool m_cursorChanged = false;
};

#endif // __KJOTSEDIT_H
/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
