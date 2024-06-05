/*
    kjots

    SPDX-FileCopyrightText: 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
    SPDX-FileCopyrightText: 2002, 2003 Aaron J. Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2003 Stanislav Kljuhhin <crz@hot.ee>
    SPDX-FileCopyrightText: 2005-2006 Jaison Lee <lee.jaison@gmail.com>
    SPDX-FileCopyrightText: 2007-2008 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef KJOTSEDIT_H
#define KJOTSEDIT_H

#include <QHelpEvent>
#include <KPIMTextEdit/RichTextComposer>

class QItemSelection;
class QItemSelectionModel;

class KActionCollection;

class KJotsEdit : public KPIMTextEdit::RichTextComposer
{
    Q_OBJECT
public:
    explicit KJotsEdit(QWidget *parent, KActionCollection *m_actionCollection);
    ~KJotsEdit();

    bool canInsertFromMimeData(const QMimeData *) const override;
    void insertFromMimeData(const QMimeData *) override;

    /**
     * Load document based on @p index
     * Index can belong to some proxy model, in that case
     * it will be mapped to the ETM
     *
     * @returns true if loaded successfully
     */
    bool setModelIndex(const QModelIndex &index);
    /**
     * Returns the current modified state of the document
     */
    bool modified();
    /**
     * Returns whether currently opened document is locked
     */
    bool locked();

    /**
     * Prepares document for saving.
     *
     * Currently it includes providing it with embedded images
     * Note: savePage calls it explicitly
     */
    void prepareDocumentForSaving();

    void createActions(KActionCollection *ac);
    void setEnableActions(bool enable);
protected:
    /* To be able to change cursor when hovering over links */
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

    /** Override to make ctrl+click follow links */
    void mousePressEvent(QMouseEvent *) override;

    void contextMenuEvent(QContextMenuEvent *event) override;

    void focusOutEvent(QFocusEvent *) override;

    bool event(QEvent *event) override;

    void tooltipEvent(QHelpEvent *event);

public Q_SLOTS:
    void onAutoBullet();
    void onLinkify(void);
    void onAutoDecimal(void);
    void DecimalList(void);

    void savePage();
    void copySelectionIntoTitle();

Q_SIGNALS:
    void linkClicked(const QUrl &url);
    void documentModified(bool modified);
private:
    class Private;
    std::unique_ptr<Private> const d;

    void createAutoDecimalList();
    KActionCollection *m_actionCollection;
    bool allowAutoDecimal;

    bool m_cursorChanged = false;
};

#endif // __KJOTSEDIT_H
/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
