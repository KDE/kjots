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

EXPORT_KONTACT_PLUGIN(KJotsPlugin, kjots)

KJotsPlugin::KJotsPlugin(KontactInterface::Core *core, const QVariantList &)
    : KontactInterface::Plugin(core, core, "kjots")
{
    setComponentName(QStringLiteral("kjots"), QStringLiteral("kjots"));

    mUniqueAppWatcher = new KontactInterface::UniqueAppWatcher(
        new KontactInterface::UniqueAppHandlerFactory<KJotsUniqueAppHandler>(), this);
}

KJotsPlugin::~KJotsPlugin()
{
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
    return { QStringLiteral("new_page"), QStringLiteral("new_book") };
}

KParts::ReadOnlyPart *KJotsPlugin::createPart()
{
    KParts::ReadOnlyPart *part = loadPart();
    if (!part) {
        return Q_NULLPTR;
    }

    return part;
}

QStringList KJotsPlugin::configModules() const
{
    QStringList modules;
    modules << QStringLiteral("PIM/kjots.desktop");
    return modules;
}


void KJotsUniqueAppHandler::loadCommandLineOptions(QCommandLineParser *parser)
{
    Q_UNUSED(parser);
}


int KJotsUniqueAppHandler::activate(const QStringList &args, const QString &workingDir)
{
    // Ensure part is loaded
    (void)plugin()->part();

    return KontactInterface::UniqueAppHandler::activate(args, workingDir);
}
#include "kjots_plugin.moc"

