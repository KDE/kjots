/*
    This file is part of KJots.

    SPDX-FileCopyrightText: 2009 Montel Laurent <montel@kde.org>
                  2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kjotsconfigdlg.h"
#include "KJotsSettings.h"
#include <QPushButton>

KJotsConfigMisc::KJotsConfigMisc(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
      ,
      ui(new Ui::confPageMisc) {
  auto lay = new QHBoxLayout(widget());
  auto miscPage = new QWidget(widget());
  ui->setupUi(miscPage);
  lay->addWidget(miscPage);
  addConfig(KJotsSettings::self(), miscPage);
  load();
}

#include "moc_kjotsconfigdlg.cpp"
