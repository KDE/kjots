/*
  Copyright (C) 2016 Daniel Vrátil <dvratil@kde.org>

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

#include "kjots_plugin.h"
#include "kjotspart.h"
//#include "akregator_options.h"
//#include "partinterface.h"

#include <KontactInterface/Core>

#include <QAction>
#include <KActionCollection>
#include <KLocalizedString>
#include <QIcon>
#include <QStandardPaths>

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

KJotsPlugin::KJotsPlugin(KontactInterface::Core *core, const QVariantList &)
    : KontactInterface::Plugin(core, core, "kjots")
{
    setComponentName(QStringLiteral("kjots"), QStringLiteral("kjots"));

    mUniqueAppWatcher = new KontactInterface::UniqueAppWatcher(
        new KontactInterface::UniqueAppHandlerFactory<KJotsUniqueAppHandler>(), this);
}

void KJotsPlugin::setHelpText(QAction *action, const QString &text)
{
    action->setStatusTip(text);
    action->setToolTip(text);
    if (action->whatsThis().isEmpty()) {
        action->setWhatsThis(text);
    }
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

QStringList KJotsPlugin::configModules() const
{
    QStringList modules;
    modules << QStringLiteral("PIM/kjots.desktop");
    return modules;
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

