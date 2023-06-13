/*
    This file is part of KJots.

    SPDX-FileCopyrightText: 2009 Montel Laurent <montel@kde.org>
                  2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kjotsconfigdlg.h"
#include "KJotsSettings.h"
#include <QPushButton>

#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
KJotsConfigMisc::KJotsConfigMisc(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
#else
KJotsConfigMisc::KJotsConfigMisc(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
#endif
      ,
      ui(new Ui::confPageMisc) {
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
  auto lay = new QHBoxLayout(this);
  auto miscPage = new QWidget(this);
#else
  auto lay = new QHBoxLayout(widget());
  auto miscPage = new QWidget(widget());
#endif
  ui->setupUi(miscPage);
  lay->addWidget(miscPage);
  addConfig(KJotsSettings::self(), miscPage);
  load();
}

#include "moc_kjotsconfigdlg.cpp"
