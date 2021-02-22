/*
    This file is part of KJots.

    SPDX-FileCopyrightText: 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
                  2002, 2003 Aaron J. Seigo <aseigo@kde.org>
                  2003 Stanislav Kljuhhin <crz@hot.ee>
                  2005-2006 Jaison Lee <lee.jaison@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "KJotsMain.h"
#include "aboutdata.h"

#include <KontactInterface/PimUniqueApplication>

#include <QCommandLineParser>
#include <QDebug>

#include <KLocalizedString>
#include <KSharedConfig>


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

    auto *jots = new KJotsMain;
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
