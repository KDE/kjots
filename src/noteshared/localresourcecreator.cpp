/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

  based on localresourcecreator from kjots

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "localresourcecreator.h"

#include <AkonadiCore/AgentManager>
#include <AkonadiCore/AgentInstanceCreateJob>
#include <AkonadiCore/ResourceSynchronizationJob>
#include <Akonadi/Notes/NoteUtils>

#include <KLocalizedString>

#include "maildirsettings.h"
#include "noteshared_debug.h"

using namespace NoteShared;

LocalResourceCreator::LocalResourceCreator(QObject *parent)
    : QObject(parent)
{

}

QString LocalResourceCreator::akonadiNotesInstanceName()
{
    return QStringLiteral("akonadi_akonotes_resource");
}

void LocalResourceCreator::createIfMissing()
{
    const Akonadi::AgentInstance::List instances = Akonadi::AgentManager::self()->instances();
    const bool found = std::any_of(instances.cbegin(), instances.cend(), [](const Akonadi::AgentInstance &instance) {
                                    return instance.type().identifier() == akonadiNotesInstanceName();
                                });
    if (found) {
        deleteLater();
        return;
    }
    createInstance();
}

void LocalResourceCreator::createInstance()
{
    Akonadi::AgentType notesType = Akonadi::AgentManager::self()->type(akonadiNotesInstanceName());

    auto *job = new Akonadi::AgentInstanceCreateJob(notesType);
    connect(job, &Akonadi::AgentInstanceCreateJob::result, this, &LocalResourceCreator::slotInstanceCreated);

    job->start();
}

void LocalResourceCreator::slotInstanceCreated(KJob *job)
{
    if (job->error()) {
        qCWarning(NOTESHARED_LOG) << job->errorString();
        deleteLater();
        return;
    }

    auto *createJob = qobject_cast<Akonadi::AgentInstanceCreateJob *>(job);
    Akonadi::AgentInstance instance = createJob->instance();

    instance.setName(i18nc("Default name for resource holding notes", "Local Notes"));
    auto *iface = new org::kde::Akonadi::Maildir::Settings(
        QStringLiteral("org.freedesktop.Akonadi.Resource.") + instance.identifier(),
        QStringLiteral("/Settings"), QDBusConnection::sessionBus(), this);

    // TODO: Make errors user-visible.
    if (!iface->isValid()) {
        qCWarning(NOTESHARED_LOG) << "Failed to obtain D-Bus interface for remote configuration.";
        delete iface;
        deleteLater();
        return;
    }
    delete iface;
    instance.reconfigure();

    auto *syncJob = new Akonadi::ResourceSynchronizationJob(instance, this);
    connect(syncJob, &Akonadi::ResourceSynchronizationJob::result, this, &LocalResourceCreator::slotSyncDone);
    syncJob->start();
}

void LocalResourceCreator::slotSyncDone(KJob *job)
{
    if (job->error()) {
        qCWarning(NOTESHARED_LOG) << "Synchronizing the resource failed:" << job->errorString();
        deleteLater();
        return;
    }

    qCWarning(NOTESHARED_LOG) << "Instance synchronized";

}

void LocalResourceCreator::finishCreateResource()
{
    deleteLater();
}

