/*
  This file is part of KJots.

  SPDX-FileCopyrightText: 2016 Daniel Vrátil <dvratil@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "kjots_plugin.h"
#include "kjotspart.h"
//#include "akregator_options.h"
//#include "partinterface.h"

#include <KontactInterface/Core>

#include <QAction>
#include <QIcon>
#include <QStandardPaths>

#include <KActionCollection>
#include <KLocalizedString>


EXPORT_KONTACT_PLUGIN_WITH_JSON(KJotsPlugin, "kjotsplugin.json")

KJotsPlugin::KJotsPlugin(KontactInterface::Core *core, const KPluginMetaData &md, const QVariantList &/*args*/)
    : KontactInterface::Plugin(core, core, md, "kjots")
{
    setComponentName(QStringLiteral("kjots"), i18n("KJots"));

    mUniqueAppWatcher = new KontactInterface::UniqueAppWatcher(
        new KontactInterface::UniqueAppHandlerFactory<KJotsUniqueAppHandler>(), this);
}

bool KJotsPlugin::isRunningStandalone() const
{
    return mUniqueAppWatcher->isRunningStandalone();
}

QStringList KJotsPlugin::invisibleToolbarActions() const
{
    return { QStringLiteral("akonadi_note_create"), QStringLiteral("akonadi_collection_create") };
}

KParts::Part *KJotsPlugin::createPart()
{
    return loadPart();
}

void KJotsUniqueAppHandler::loadCommandLineOptions(QCommandLineParser *parser)
{
    Q_UNUSED(parser)
}


int KJotsUniqueAppHandler::activate(const QStringList &args, const QString &workingDir)
{
    // Ensure part is loaded
    (void)plugin()->part();

    return KontactInterface::UniqueAppHandler::activate(args, workingDir);
}
#include "kjots_plugin.moc"


#include "moc_kjots_plugin.cpp"
