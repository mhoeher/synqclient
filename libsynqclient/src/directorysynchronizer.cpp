/*
 * Copyright 2020-2021 Martin Hoeher <martin@rpdev.net>
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

#include "SynqClient/directorysynchronizer.h"

#include <QFileInfo>
#include <QTimer>

#include "SynqClient/abstractjobfactory.h"
#include "SynqClient/syncstatedatabase.h"

#include "directorysynchronizerprivate.h"

namespace SynqClient {

/**
 * @class DirectorySynchronizer
 * @brief Synchronizes a local and a remote directory.
 *
 * This class is used to keep two directories - a local one and a remote one - synchronized.
 *
 * # Synchronizer Life Cycle
 *
 * The usage pattern of the synchronizer is actually quite similar to that of jobs:
 *
 * After creating a new instance of this class, it is in the SynchronizerState::Ready state. In this
 * state, it must be set up, i.e. at least the following attributes must be set:
 *
 * - jobFactory()
 * - syncStateDatabase()
 * - localDirectoryPath()
 * - remoteDirectoryPath()
 *
 * In addition, other properties can be changed to alter the bahviour of the synchronization.
 *
 * Once all inputs are provided, the start() method can be called. This will transition the
 * synchronizer to the SynchronizerState::Running state. The method returns immediately. Eventually,
 * the finished() signal will be emitted to indicate that the sync is done and the synchronizer
 * transitioned to the SynchronizerState::Finished state.
 *
 * While the synchronizer is running, the stop() method can be used to stop the synchronization.
 *
 * # Error Handling
 *
 * This is also similar to the job interface: The synchronizer provides the error() method, which
 * returns an error ID indicating any issues occurred during the synchronization. Note that only the
 * first error that occurred will be reported by that. This is important to keep in mind, as the
 * synchronizer will spawn multiple jobs in parallel to improve overall throughput and make use of
 * pipelining when using network jobs.
 *
 * In addition, the errorString() method can be used to retrieve a textual representation of the
 * error that occurred. This might also contain valuable information from underlying jobs that
 * failed.
 */

/**
 * @brief Constructor.
 */
DirectorySynchronizer::DirectorySynchronizer(QObject* parent)
    : QObject(parent), d_ptr(new DirectorySynchronizerPrivate(this))
{
    qRegisterMetaType<SynchronizerLogEntryType>();
    connect(this, &DirectorySynchronizer::finished, this, [=]() {
        emit logMessageAvailable(SynchronizerLogEntryType::Information,
                                 tr("Finished synchronization"));
    });
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
 * path to the file or folder and a FileInfo object and returns either true if that file/folder
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
 * will establish at most 6 connections to the same host/port pair in parallel. As jobs might
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
 * @brief Indicates that the sync should be retried with fewer parallel jobs.
 *
 * If this flag is set after a sync, this indicate that the remote had issues and hence we could
 * potentially retry the sync with fewer parallel workers (if possible). Users of the class should
 * check if the sync finished with this error and retry it, with the maximum number of parallel jobs
 * reduced (in the worst case to e.g. only 1 worker). This sometimes is necessary when the
 * server used is not capable to handle too many requests in parallel and hence starts closing
 * connections prematurely.
 *
 * @note Do not reuse the same DirectorySynchronizer object. Each object can be used only once,
 * hence, a new object must be created for the retry.
 */
bool DirectorySynchronizer::retryWithFewerJobs() const
{
    Q_D(const DirectorySynchronizer);
    return d->retryWithFewerJobs;
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
 * @brief Get a textual error description for the last error.
 */
QString DirectorySynchronizer::errorString() const
{
    Q_D(const DirectorySynchronizer);
    return d->errorString;
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

    auto progressTimer = new QTimer(this);
    progressTimer->setInterval(100);
    progressTimer->setSingleShot(false);
    connect(progressTimer, &QTimer::timeout, this, [=]() { emit progress(d->progress); });
    progressTimer->start();

    emit logMessageAvailable(SynchronizerLogEntryType::Information, tr("Starting synchronization"));

    d->state = SynchronizerState::Running;

    if (!d->jobFactory || !d->syncStateDatabase || !d->filter || d->localDirectoryPath.isEmpty()
        || d->remoteDirectoryPath.isEmpty()) {
        d->setError(SynchronizerError::MissingParameter, tr("Some parameters are missing"),
                    JobError::NoError);
        return;
    }

    if (d->maxJobs < 1) {
        d->setError(SynchronizerError::InvalidParameter,
                    tr("The maximum number of jobs must be at least 1"), JobError::NoError);
        return;
    }

    if (!QFileInfo(d->localDirectoryPath).isDir()) {
        d->setError(SynchronizerError::MissingParameter,
                    tr("The local directory to be synced must exist"), JobError::NoError);
        return;
    }

    if (!d->syncStateDatabase->openDatabase()) {
        d->setError(SynchronizerError::FailedOpeningSyncStateDatabase,
                    tr("Failed to open the sync state database"), JobError::NoError);
    }

    // Check if the CreateRemoteFolderOnFirstSync flag is unset. If so, do not try to create the
    // remote root folder but right away run the sync.
    if (!d->flags.testFlag(SynchronizerFlag::CreateRemoteFolderOnFirstSync)) {
        d->createSyncPlan();
    } else {
        // Check if this is the first time sync. If so, ensure the remote folder
        // exists:
        auto entry = d->syncStateDatabase->getEntry("/");
        if (!entry.isValid()) {
            d->createRemoteFolder();
        } else {
            d->createSyncPlan();
        }
    }
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
    Q_D(DirectorySynchronizer);
    d->setError(SynchronizerError::Stopped, "The sync has been stopped", JobError::NoError);
    d->stopped = true;
    emit d->stopRequested();
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
 * @fn DirectorySynchronizer::logMessageAvailable()
 * @brief Used to report messages during the sync.
 *
 * This signal is emitted when the synchronizer runs a particular action or encounters an issue.
 * The @p type indicates the type of message, e.g. if it is an informational message, a file is
 * being downloaded or a remote folder being created.
 *
 * Depending on the concrete type, @p message is either an arbitrary string (containing more details
 * about the issue) or the path (relative to the local and remote root folder) which is being
 * affected.
 */

/**
 * @fn DirectorySynchronizer::progress(int value)
 * @brief Indicate progress of the sync operation.
 *
 * This signal is emitted from time to time to inform about the overall progress of
 * the sync operation. A negative value indicates unknown progress (this might be
 * reported in the initial phase when the sync plan is being created). As soon as
 * the known steps are gathered, this signal is emitted with values between
 * 0 and 100, indicating the overall progress of the operation.
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
