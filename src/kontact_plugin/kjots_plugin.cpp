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

#if KONTACTINTERFACE_VERSION < QT_VERSION_CHECK(5, 14, 42)
/**
  Exports Kontact plugin.
  @param pluginclass the class to instantiate (must derive from KontactInterface::Plugin
  @param jsonFile filename of the JSON file, generated from a .desktop file
 */
#define EXPORT_KONTACT_PLUGIN_WITH_JSON( pluginclass, jsonFile ) \
    class Instance                                           \
    {                                                        \
    public:                                                \
        static QObject *createInstance( QWidget *, QObject *parent, const QVariantList &list ) \
        { return new pluginclass( static_cast<KontactInterface::Core*>( parent ), list ); } \
    };                                                                    \
    K_PLUGIN_FACTORY_WITH_JSON( KontactPluginFactory, jsonFile, registerPlugin< pluginclass >   \
                              ( QString(), Instance::createInstance ); ) \
    K_EXPORT_PLUGIN_VERSION(KONTACT_PLUGIN_VERSION)
#endif

EXPORT_KONTACT_PLUGIN_WITH_JSON(KJotsPlugin, "kjotsplugin.json")

KJotsPlugin::KJotsPlugin(KontactInterface::Core *core, const QVariantList &/*args*/)
    : KontactInterface::Plugin(core, core, "kjots")
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

#if KONTACTINTERFACE_VERSION >= QT_VERSION_CHECK(5, 14, 42)
KParts::Part *KJotsPlugin::createPart()
{
    return loadPart();
}
#else
KParts::ReadOnlyPart *KJotsPlugin::createPart()
{
    return loadPart();
}
#endif

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

