/*
    This file is part of KJots.

    SPDX-FileCopyrightText: 2009 Montel Laurent <montel@kde.org>
                  2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KJOTSCONFIGDLG_H
#define KJOTSCONFIGDLG_H
#include "kcmutils_version.h"
#include "ui_confpagemisc.h"
#include <KCModule>
#include <KCMultiDialog>

class KJotsConfigMisc : public KCModule
{
    Q_OBJECT

public:
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
  explicit KJotsConfigMisc(QWidget *parent, const QVariantList &args = {});
#else
  explicit KJotsConfigMisc(QObject *parent, const KPluginMetaData &data = {});
#endif
private:
    std::unique_ptr<Ui::confPageMisc> ui;
};

#endif /* KJOTSCONFIGDLG_H */

