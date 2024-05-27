/*
  This file is part of KJots.

  SPDX-FileCopyrightText: 2008 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "aboutdata.h"
#include "kjots-version.h"

#include <KLocalizedString>

AboutData::AboutData()
    : KAboutData(QStringLiteral("kjots"),
                 i18n("KJots"),
                 QStringLiteral(KJOTS_VERSION),
                 i18n("KDE note taking utility"),
                 KAboutLicense::GPL,
                 i18n("Copyright © 1997–2024 KJots authors"))
{
    addAuthor(i18nc("@info:credit", "Igor Poboiko"), i18n("Maintainer"), QStringLiteral("igor.poboiko@gmail.com"));
    addAuthor(i18nc("@info:credit", "Daniel Vrátil"), i18n("Port to KDE Frameworks 5"), QStringLiteral("dvraitl@kde.org"));
    addAuthor(i18nc("@info:credit", "Stephen Kelly"), QString(), QStringLiteral("steveire@gmail.com"));
    addAuthor(i18nc("@info:credit", "Pradeepto K. Bhattacharya"), QString(), QStringLiteral("pradeepto@kde.org"));
    addAuthor(i18nc("@info:credit", "Jaison Lee"), QString(), QStringLiteral("lee.jaison@gmail.com"));
    addAuthor(i18nc("@info:credit", "Aaron J. Seigo"), QString(), QStringLiteral("aseigo@kde.org"));
    addAuthor(i18nc("@info:credit", "Stanislav Kljuhhin"), QString(), QStringLiteral("crz@starman.ee"));
    addAuthor(i18nc("@info:credit", "Christoph Neerfeld"), i18n("Original author"), QStringLiteral("chris@kde.org"));
    addAuthor(i18nc("@info:credit", "Laurent Montel"), QString(), QStringLiteral("montel@kde.org"));
}
