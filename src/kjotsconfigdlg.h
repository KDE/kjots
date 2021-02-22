/*
    This file is part of KJots.

    SPDX-FileCopyrightText: 2009 Montel Laurent <montel@kde.org>
                  2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KJOTSCONFIGDLG_H
#define KJOTSCONFIGDLG_H

#include <KCModule>
#include <KCMultiDialog>
#include "ui_confpagemisc.h"

class KJotsConfigMisc : public KCModule
{
    Q_OBJECT

public:
    explicit KJotsConfigMisc(QWidget *parent, const QVariantList &args = QVariantList());
private:
    std::unique_ptr<Ui::confPageMisc> ui;
};

#endif /* KJOTSCONFIGDLG_H */

