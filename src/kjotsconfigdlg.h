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
  explicit KJotsConfigMisc(QObject *parent, const KPluginMetaData &data = {});
private:
    std::unique_ptr<Ui::confPageMisc> ui;
};

#endif /* KJOTSCONFIGDLG_H */

