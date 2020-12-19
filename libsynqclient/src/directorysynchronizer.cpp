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
 * path to the file or folder and returns either true if that file/folder shall be included in the
 * synchronization or false otherwise. The paths passed to the filter are formatted as absolute
 * paths with forward slashes. The path is implicitly relative to the configured local and remote
 * directory.
 *
 * The default filter returns true for every path passed into it.
 *
 * @note If you plan to put the data files for the syncStateDatabase() in the local folder
 * that is synchronized, you have to configure a suitable filter as well. Otherwise, the
 * synchronization database is synced as well - which is almost certainly never what you intent.
 *
 * For example, if you plan to use the JSONSyncStateDatabase and you want to store it as
 * `sync.json` inside the folder that is synchronized, you can use the following to exclude the
 * databse from sync:
 *
 * @code
 * DirectorySynchronizer sync;
 * sync.setFilter([](const QString &path) {
 *     if (path == "/sync.json") {
 *         // Exclude the sync state database file from the sync:
 *         return false;
 *     }
 *     // Include every other file:
 *     return true;
 * });
 * @endcode
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
 * @brief Constructor.
 */
DirectorySynchronizer::DirectorySynchronizer(DirectorySynchronizerPrivate* d, QObject* parent)
    : QObject(parent), d_ptr(d)
{
}

} // namespace SynqClient
