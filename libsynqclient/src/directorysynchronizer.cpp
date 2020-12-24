/*
 * Copyright 2020 Martin Hoeher <martin@rpdev.net>
 *
 * This file is part of SynqClient.
 *
 * SynqClient is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * SynqClient is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SynqClient.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../inc/directorysynchronizer.h"

#include <QFileInfo>

#include "abstractjobfactory.h"
#include "syncstatedatabase.h"

#include "directorysynchronizerprivate.h"

namespace SynqClient {

/**
 * @class DirectorySynchronizer
 * @brief Synchronizes a local and a remote directory.
 *
 * This class is used to keep two directories - a local one and a remote one - synchronized.
 *
 */
/**
 * @brief Constructor.
 */
DirectorySynchronizer::DirectorySynchronizer(QObject* parent)
    : QObject(parent), d_ptr(new DirectorySynchronizerPrivate(this))
{
}

/**
 * @brief Destructor.
 */
DirectorySynchronizer::~DirectorySynchronizer() {}

/**
 * @brief The factory used to created jobs to access remote files.
 *
 * This returns the factory which is used to create the jobs necessary to access remote files and
 * folders. By default, this is a nullptr. Before using the synchronizer, this must be set to a
 * valid factory.
 */
AbstractJobFactory* DirectorySynchronizer::jobFactory() const
{
    Q_D(const DirectorySynchronizer);
    return d->jobFactory;
}

/**
 * @brief Set the factory used to create jobs for accessing remote files.
 */
void DirectorySynchronizer::setJobFactory(AbstractJobFactory* jobFactory)
{
    Q_D(DirectorySynchronizer);
    d->jobFactory = jobFactory;
}

/**
 * @brief The persistent sync state storage.
 *
 * This holds the database used to persistently store sync state information. This is required
 * to detect both local and remote changes between sync runs. By default, this is a nullptr and
 * must be set to a valid database before using the synchronizer.
 */
SyncStateDatabase* DirectorySynchronizer::syncStateDatabase() const
{
    Q_D(const DirectorySynchronizer);
    return d->syncStateDatabase;
}

/**
 * @brief Set the persistent sync state storage.
 */
void DirectorySynchronizer::setSyncStateDatabase(SyncStateDatabase* syncStateDatabase)
{
    Q_D(DirectorySynchronizer);
    d->syncStateDatabase = syncStateDatabase;
}

/**
 * @brief The path to the local directory to sync.
 *
 * This is the path to the local directory which shall be synchronized. By default, this is
 * an empty string. To use the synchronizer, set it to a valid folder path.
 *
 * @note The local directory must exist.
 */
QString DirectorySynchronizer::localDirectoryPath() const
{
    Q_D(const DirectorySynchronizer);
    return d->localDirectoryPath;
}

/**
 * @brief Set the path to the local folder to be synchronized.
 */
void DirectorySynchronizer::setLocalDirectoryPath(const QString& localDirectoryPath)
{
    Q_D(DirectorySynchronizer);
    d->localDirectoryPath = localDirectoryPath;
}

/**
 * @brief The path to the remote directory to be synchronized.
 *
 * This is the path to the remote path that shall be synchronized. Please note that the path
 * specified might be relative to a *root folder* which is part of the configuration of the
 * used jobFactory().
 *
 * By default, this is empty. Set it to a valid string. If you need to synchronize against the
 * root folder configured in the job factory, use `/`.
 */
QString DirectorySynchronizer::remoteDirectoryPath() const
{
    Q_D(const DirectorySynchronizer);
    return d->remoteDirectoryPath;
}

/**
 * @brief Set the path to the remote directory to be synchronized.
 */
void DirectorySynchronizer::setRemoteDirectoryPath(const QString& remoteDirectoryPath)
{
    Q_D(DirectorySynchronizer);
    d->remoteDirectoryPath = remoteDirectoryPath;
}

/**
 * @brief A filter determining files and folders to be included in the synchronization.
 *
 * This returns the filter that is used during the synchronization to determine which files
 * and folders to include in the synchronization. The filter is a simple function which gets the
 * path to the file or folder and the FileInfo object and returns either true if that file/folder
 * shall be included in the synchronization or false otherwise. The paths passed to the filter are
 * formatted as absolute paths with forward slashes. The path is implicitly relative to the
 * configured local and remote directory.
 *
 * The default filter returns true for every path passed into it.
 *
 * If you plan to put the data files for the syncStateDatabase() in the local folder
 * that is synchronized, you have to configure a suitable filter as well. Otherwise, the
 * synchronization database is synced as well - which is almost certainly never what you intent.
 *
 * For example, if you plan to use the JSONSyncStateDatabase and you want to store it as
 * `sync.json` inside the folder that is synchronized, you can use the following to exclude the
 * databse from sync:
 *
 * @code
 * DirectorySynchronizer sync;
 * sync.setFilter([](const QString &path, const SynqClient::FileInfo &fileInfo) {
 *     if (path == "/sync.json") {
 *         // Exclude the sync state database file from the sync:
 *         return false;
 *     }
 *     // Include every other file:
 *     return true;
 * });
 * @endcode
 *
 * @sa SyncStateEntry::makePath()
 */
DirectorySynchronizer::Filter DirectorySynchronizer::filter() const
{
    Q_D(const DirectorySynchronizer);
    return d->filter;
}

/**
 * @brief Set the filter used to determine which files and folders to include in the sync.
 */
void DirectorySynchronizer::setFilter(const DirectorySynchronizer::Filter& filter)
{
    Q_D(DirectorySynchronizer);
    d->filter = filter;
}

/**
 * @brief The maximal number of jobs to spawn in parallel.
 *
 * This is the maximal number of jobs that are created in parallel during the sync. By
 * default, this is set to 12. This is in line with the QNetworkAccessManager API: Internally, it
 * will establish at most 6 connections to to the same host/port pair in parallel. As jobs might
 * have some "dead" time during which no other network connection takes place, we use double
 * that number of jobs to ensure that we utilize parallelization and pipelining in an optimal
 * way to speed up network transfers.
 *
 * When using a job factory which does not internally use QNetworkAccessManager, adjust this
 * value accordingly. In particular, in case you have a job implementation which requires
 * sequential access, you should set this value to 1.
 */
int DirectorySynchronizer::maxJobs() const
{
    Q_D(const DirectorySynchronizer);
    return d->maxJobs;
}

/**
 * @brief Set the maximal number of jobs to spawn in parallel.
 */
void DirectorySynchronizer::setMaxJobs(int maxJobs)
{
    Q_D(DirectorySynchronizer);
    d->maxJobs = maxJobs;
}

/**
 * @brief The strategy to be used in case a sync conflict is detected.
 *
 * This determines how the synchronizer proceeds in case it detects a sync conflict.
 */
SyncConflictStrategy DirectorySynchronizer::syncConflictStrategy() const
{
    Q_D(const DirectorySynchronizer);
    return d->syncConflictStrategy;
}

/**
 * @brief Set the @p strategy to be used when a sync conflict is detected.
 */
void DirectorySynchronizer::setSyncConflictStrategy(SyncConflictStrategy strategy)
{
    Q_D(DirectorySynchronizer);
    d->syncConflictStrategy = strategy;
}

/**
 * @brief Settings to fine tune the synchronization.
 *
 * This returns the flags which control some aspects of the sync. By default,
 * this is set to SynchronizerFlag::DefaultFlags.
 */
SynchronizerFlags DirectorySynchronizer::flags() const
{
    Q_D(const DirectorySynchronizer);
    return d->flags;
}

/**
 * @brief Set the @p flags which control some of the behavior of the sync.
 */
void DirectorySynchronizer::setFlags(const SynchronizerFlags flags)
{
    Q_D(DirectorySynchronizer);
    d->flags = flags;
}

/**
 * @brief The state of the synchronizer.
 *
 * This returns the current state of the synchronizer. After creation, the synchronizer is in
 * the SynchronizerState::Ready state. In this state, the start() method can be called to
 * trigger the synchronization. This causes the synchronizer to change into the
 * SynchronizerState::Running state. As soon as the sync is done, the synchronizer changes into the
 * SynchronizerState::Finished state.
 */
SynchronizerState DirectorySynchronizer::state() const
{
    Q_D(const DirectorySynchronizer);
    return d->state;
}

/**
 * @brief Indicates the status of the finished synchronization.
 *
 * This returns the error reason for the sync. If the sync was successful, this is
 * SynchronizerError::NoError. Otherwise, this is set to a value indicating the reason why the sync
 * was not successful.
 */
SynchronizerError DirectorySynchronizer::error() const
{
    Q_D(const DirectorySynchronizer);
    return d->error;
}

/**
 * @brief Start the synchronization.
 *
 * This starts synchronizing the configured local and remote folders. Calling this will cause the
 * synchronizer to change into the SynchronizerState::Running state. Eventually, the synchronization
 * will finish. This is indicated by emitting the finished() signal. It will then switch to the
 * SynchronizerState::Finished state.
 *
 * @note Calling this method has no effect if the synchronizer is not in the
 * SynchronizerState::Ready state.
 */
void DirectorySynchronizer::start()
{
    Q_D(DirectorySynchronizer);
    if (d->state != SynchronizerState::Ready) {
        return;
    }

    d->state = SynchronizerState::Running;

    if (!d->jobFactory || !d->syncStateDatabase || !d->filter || d->localDirectoryPath.isEmpty()
        || d->remoteDirectoryPath.isEmpty()) {
        d->error = SynchronizerError::MissingParameter;
        d->finishLater();
        return;
    }

    if (d->maxJobs < 1) {
        d->error = SynchronizerError::InvalidParameter;
        d->finishLater();
        return;
    }

    if (!QFileInfo(d->localDirectoryPath).isDir()) {
        d->error = SynchronizerError::InvalidParameter;
        d->finishLater();
        return;
    }

    d->finishLater();
}

/**
 * @brief Stop the running synchronization.
 *
 * This method can be used to stop the running synchronization. This will cause currently running
 * jobs to be stopped. The overall sync will then be finished with a SynchronizerError::Stopped
 * error.
 *
 * @note Calling this method has no effect, if the synchronizer is not in the
 * SynchronizerState::Running state.
 */
void DirectorySynchronizer::stop()
{
    // TODO
}

/**
 * @brief Constructor.
 */
DirectorySynchronizer::DirectorySynchronizer(DirectorySynchronizerPrivate* d, QObject* parent)
    : QObject(parent), d_ptr(d)
{
}

/**
 * @fn DirectorySynchronizer::finished()
 * @brief Synchronization has finished.
 *
 * This signal is emitted to indicate that the sync has finished - independent on whether it was
 * successful or not. Check the value returned by the error() function to learn if the sync finished
 * with a problem or not.
 */

/**
 * @typedef DirectorySynchronizer::Filter
 * @brief Type definition for file filters.
 *
 * This typedef defines the signature of callables suitable to be used as file filters. A filter
 * function gets two parameters:
 *
 * - The path to the file or folder. This is usually formatted as an absolute path with forward
 *   slashes only. This path is interpreted to be relative to the local and/or remote folder
 *   which is synchronized.
 * - The FileInfo record, which potentially holds more information about the file or folder.
 *   In particular, it holds the information if a path refers to a file or a folder.
 *
 * The function shall return true if the file or folder shall be included in the sync.
 *
 * @note If for a folder the filter returns false, the sync will exclude all files and folders
 * below that one recursively. Hence, if you need to skip e.g. files directly below a folder but
 * include ones further down the same hierarchy, you must return true for the folder itself.
 */

} // namespace SynqClient
