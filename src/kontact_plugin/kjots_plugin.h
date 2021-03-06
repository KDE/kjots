/*
  SPDX-FileCopyrightText: 2016 Daniel Vrátil <dvratil@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef KJOTS_PLUGIN_H
#define KJOTS_PLUGIN_H

#include <KontactInterface/UniqueAppHandler>
#include <kontactinterface_version.h>

namespace KontactInterface
{
class Plugin;
}

class KJotsUniqueAppHandler : public KontactInterface::UniqueAppHandler
{
    Q_OBJECT
public:
    explicit KJotsUniqueAppHandler(KontactInterface::Plugin *plugin)
        : KontactInterface::UniqueAppHandler(plugin)
    {
    }

    int activate(const QStringList &args, const QString &workingDir) Q_DECL_OVERRIDE;
    void loadCommandLineOptions(QCommandLineParser *parser) Q_DECL_OVERRIDE;

};

class KJotsPlugin : public KontactInterface::Plugin
{
    Q_OBJECT

public:
    KJotsPlugin(KontactInterface::Core *core, const QVariantList &);

    int weight() const Q_DECL_OVERRIDE
    {
        return 475;
    }

    bool isRunningStandalone() const Q_DECL_OVERRIDE;

    QStringList invisibleToolbarActions() const Q_DECL_OVERRIDE;

protected:
#if KONTACTINTERFACE_VERSION >= QT_VERSION_CHECK(5, 14, 42)
    KParts::Part *createPart() override;
#else
    KParts::ReadOnlyPart *createPart() Q_DECL_OVERRIDE;
#endif
    KontactInterface::UniqueAppWatcher *mUniqueAppWatcher;
};

#endif

