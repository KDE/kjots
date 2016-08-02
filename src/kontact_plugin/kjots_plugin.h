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

  You should have received a copy of the GNU General Public Licensea along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef KJOTS_PLUGIN_H
#define KJOTS_PLUGIN_H

#include <KontactInterface/UniqueAppHandler>

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
    ~KJotsPlugin();

    int weight() const Q_DECL_OVERRIDE
    {
        return 475;
    }

    virtual QStringList configModules() const;
    bool isRunningStandalone() const Q_DECL_OVERRIDE;

    QStringList invisibleToolbarActions() const Q_DECL_OVERRIDE;

protected:
    KParts::ReadOnlyPart *createPart() Q_DECL_OVERRIDE;
    KontactInterface::UniqueAppWatcher *mUniqueAppWatcher;

private:
    void setHelpText(QAction *action, const QString &text);
};

#endif

