/*
  This file is part of KJots.

  Copyright (c) 2008 Stephen Kelly <steveire@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
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
                 i18n("Copyright © 1997–2015 KJots authors"))
{
    addAuthor(i18n("Daniel Vrátil"), i18n("Maintainer"), QStringLiteral("dvraitl@kde.org"));
    addAuthor(i18n("Stephen Kelly"), QString(), QStringLiteral("steveire@gmail.com"));
    addAuthor(i18n("Pradeepto K. Bhattacharya"), QString(), QStringLiteral("pradeepto@kde.org"));
    addAuthor(i18n("Jaison Lee"), QString(), QStringLiteral("lee.jaison@gmail.com"));
    addAuthor(i18n("Aaron J. Seigo"), QString(), QStringLiteral("aseigo@kde.org"));
    addAuthor(i18n("Stanislav Kljuhhin"), QString(), QStringLiteral("crz@starman.ee"));
    addAuthor(i18n("Christoph Neerfeld"), i18n("Original author"), QStringLiteral("chris@kde.org"));
    addAuthor(i18n("Laurent Montel"), QString(), QStringLiteral("montel@kde.org"));
}
