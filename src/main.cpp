// -*- C++ -*-

//
//  kjots
//
//  Copyright (C) 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
//  Copyright (C) 2002, 2003 Aaron J. Seigo <aseigo@kde.org>
//  Copyright (C) 2003 Stanislav Kljuhhin <crz@hot.ee>
//  Copyright (C) 2005-2006 Jaison Lee <lee.jaison@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "KJotsMain.h"
#include "aboutdata.h"

#include <KontactInterface/PimUniqueApplication>

#include <QCommandLineParser>

#include <KLocalizedString>
#include <KConfigGroup>
#include <KConfig>
#include <KSharedConfig>

#include <qdebug.h>

int main(int argc, char **argv)
{
    AboutData aboutData;
    KontactInterface::PimUniqueApplication app(argc, &argv);
    KLocalizedString::setApplicationDomain("kjots");
    app.setAboutData(aboutData);

    QCommandLineParser *cmdArgs = app.cmdArgs();
    const QStringList args = QApplication::arguments();
    cmdArgs->process(args);
    aboutData.processCommandLine(cmdArgs);

    if (!KontactInterface::PimUniqueApplication::start(args)) {
        qWarning() << "kjots is already running!";
        exit(0);
    }

    KJotsMain *jots = new KJotsMain;
    if (app.isSessionRestored()) {
        if (KJotsMain::canBeRestored(1)) {
            jots->restore(1);
        }
    }

    jots->show();
    jots->resize(jots->size());
    return app.exec();
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
