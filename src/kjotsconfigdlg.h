/*
    Copyright (c) 2009 Montel Laurent <montel@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef KJOTSCONFIGDLG_H
#define KJOTSCONFIGDLG_H

#include <KCModule>
#include <KCMultiDialog>
#include "ui_confpagemisc.h"

class confPageMisc : public QWidget, public Ui::confPageMisc
{
    Q_OBJECT

public:
    explicit confPageMisc(QWidget *parent) : QWidget(parent)
    {
        setupUi(this);
    }
};

class KJotsConfigMisc : public KCModule
{
    Q_OBJECT

public:
    explicit KJotsConfigMisc(QWidget *parent, const QVariantList &args = QVariantList());

    void load() override;
    void save() override;
private Q_SLOTS:
    void modified();
private:
    confPageMisc *miscPage;
};

class KJotsConfigDlg : public KCMultiDialog
{
    Q_OBJECT

public:
    explicit KJotsConfigDlg(const QString &title, QWidget *parent);

public Q_SLOTS:
    void slotOk();
};

#endif /* KJOTSCONFIGDLG_H */

