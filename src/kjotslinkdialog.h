/*
    This file is a part of KJots.

    SPDX-FileCopyrightText: 2008 Stephen Kelly <steveire@gmail.com>
                  2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KJOTSLINKDIALOG_H
#define KJOTSLINKDIALOG_H

#include <QDialog>

namespace Ui {
    class LinkDialog;
}

class QListView;

class KDescendantsProxyModel;

class QAbstractItemModel;
class QString;


class KJotsLinkDialog : public QDialog
{
    Q_OBJECT
public:
    explicit KJotsLinkDialog(QAbstractItemModel *kjotsModel, QWidget *parent = nullptr);
    ~KJotsLinkDialog();

    /**
     * Returns the link text shown in the dialog
     * @param linkText The initial text
     */
    void setLinkText(const QString &linkText);

    /**
     * Sets the target link url shown in the dialog
     * @param linkUrl The initial link target url
     */
    void setLinkUrl(const QString &linkUrl);

    /**
     * Returns the link text entered by the user.
     * @return The link text
     */
    QString linkText() const;

    /**
     * Returns the target link url entered by the user.
     * @return The link url
     */
    QString linkUrl() const;
private:
    void slotTextChanged();
    std::unique_ptr<Ui::LinkDialog> ui;
    std::unique_ptr<KDescendantsProxyModel> m_descendantsProxyModel;
};

#endif
