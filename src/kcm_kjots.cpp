/*
    SPDX-FileCopyrightText: 2009 Montel Laurent <montel@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kjotsconfigdlg.h"

#include <KPluginFactory>

K_PLUGIN_FACTORY(KCMKJotsConfigFactory, registerPlugin<KJotsConfigMisc>();)

#include "kcm_kjots.moc"
