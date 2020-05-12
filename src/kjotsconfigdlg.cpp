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

#include "kjotsconfigdlg.h"
#include "KJotsSettings.h"
#include <QPushButton>

KJotsConfigMisc::KJotsConfigMisc(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , ui(new Ui::confPageMisc)
{
    auto *lay = new QHBoxLayout(this);
    auto miscPage = new QWidget(this);
    ui->setupUi(miscPage);
    lay->addWidget(miscPage);
    addConfig(KJotsSettings::self(), miscPage);
    load();
}

#include "moc_kjotsconfigdlg.cpp"
