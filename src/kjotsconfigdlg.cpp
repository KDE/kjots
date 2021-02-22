/*
    This file is part of KJots.

    SPDX-FileCopyrightText: 2009 Montel Laurent <montel@kde.org>
                  2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
