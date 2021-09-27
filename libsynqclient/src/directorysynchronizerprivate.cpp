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

#include "directorysynchronizerprivate.h"

#include <algorithm>

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QLoggingCategory>
#include <QQueue>
#include <QSaveFile>
#include <QThread>
#include <QTimer>

#include "SynqClient/abstractjobfactory.h"
#include "SynqClient/createdirectoryjob.h"
#include "SynqClient/deletejob.h"
#include "SynqClient/downloadfilejob.h"
#include "SynqClient/getfileinfojob.h"
#include "SynqClient/listfilesjob.h"
#include "SynqClient/syncstatedatabase.h"
#include "SynqClient/uploadfilejob.h"

namespace SynqClient {

static Q_LOGGING_CATEGORY(log, "SynqClient.DirectorySynchronizer", QtWarningMsg);

DirectorySynchronizerPrivate::DirectorySynchronizerPrivate(DirectorySynchronizer* q)
    : QObject(),
      q_ptr(q),
      jobFactory(nullptr),
      syncStateDatabase(nullptr),
      localDirectoryPath(),
      remoteDirectoryPath(),
      filter([](const QString&, const FileInfo&) { return true; }),
      state(SynchronizerState::Ready),
      error(SynchronizerError::NoError),
      errorString(),
      maxJobs(12),
      retryWithFewerJobs(false),
      syncConflictStrategy(SyncConflictStrategy::RemoteWins),
      flags(SynchronizerFlag::DefaultFlags),
      stopped(false),
      progress(-1),
      numTotalSyncActionsToRun(0),
      remoteFoldersSyncAttributes(),
      runningJobs(0),
      createdRemoteFolderParts(),
      remoteFolderPartsToCreate(),
      localChangeTree(),
      remoteChangeTree(),
      remoteFoldersToScan(),
      syncActionsToRun(),
      remoteFoldersToCreate(),
      remoteResourcesToDelete()
{
}

/**
 * @brief Set up some default job signal/slot connections.
 *
 * This mainly takes care to auto-delete jobs once they are finished as well as making sure that
 * jobs stop executing when the user requests to terminate the sync.
 */
void DirectorySynchronizerPrivate::setupDefaultJobSignals(AbstractJob* job)
{
    connect(this, &DirectorySynchronizerPrivate::stopRequested, job, &AbstractJob::stop);
    connect(job, &AbstractJob::finished, job, &QObject::deleteLater);
}

/**
 * @brief Helper function which converts a list of sync entries to a map (with paths as keys).
 */
QMap<QString, SyncStateEntry>
DirectorySynchronizerPrivate::syncStateListToMap(const QVector<SyncStateEntry>& list)
{
    QMap<QString, SyncStateEntry> result;
    for (const auto& entry : list) {
        result[entry.path()] = entry;
    }
    return result;
}

/**
 * @brief Create the next part of the path of a remote folder.
 */
void DirectorySynchronizerPrivate::createNextRemoteFolderPart()
{
    Q_Q(DirectorySynchronizer);

    if (error != SynchronizerError::NoError) {
        return;
    }

    if (remoteFolderPartsToCreate.isEmpty()) {
        // We are done creating remote folders.
        qCDebug(log) << "Finished creating remote folder structure";

        // Create a "stub" entry, so we know we already created the remote folder:
        SyncStateEntry entry("/", QDateTime(), QString());
        syncStateDatabase->addEntry(entry);

        // Start next phase:
        createSyncPlan();

        return;
    }
    auto next = remoteFolderPartsToCreate.takeFirst();
    createdRemoteFolderParts.append(next);
    auto path = "/" + createdRemoteFolderParts.join("/");
    qCDebug(log) << "Creating remote folder" << path;
    auto job = jobFactory->createDirectory(this);
    job->setPath(path);
    setupDefaultJobSignals(job);
    connect(job, &CreateDirectoryJob::finished, q, [=]() {
        switch (job->error()) {
        case JobError::NoError:
            // Creating the directory succeeded, proceed:
            createNextRemoteFolderPart();
            break;
        case JobError::FolderExists:
            // The remote folder already exists - proceed:
            createNextRemoteFolderPart();
            break;
        default:
            setError(SynchronizerError::FailedCreatingRemoteFolder,
                     tr("Failed creating remote directory: %1").arg(job->errorString()),
                     job->error());
        }
    });
    job->start();
}

ChangeTree DirectorySynchronizerPrivate::buildLocalChangeTree()
{
    ChangeTree result;
    QQueue<QString> paths;
    paths.enqueue("/");
    while (!paths.isEmpty()) {
        auto path = paths.dequeue();
        bool innerOk;
        auto previousEntries = syncStateDatabase->findEntries(path, &innerOk);
        if (!innerOk) {
            setError(SynchronizerError::SyncStateDatabaseLookupFailed,
                     tr("Failed to read sync state database for entry %1").arg(path),
                     JobError::NoError);
            break;
        }
        auto previousEntriesMap = syncStateListToMap(previousEntries);
        auto localPath = QDir::cleanPath(localDirectoryPath + "/" + path);
        QSet<QString> handledEntries;
        QDir dir(localPath);
        QDir rootDir(localDirectoryPath);
        auto entryInfoList = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        for (const auto& entry : qAsConst(entryInfoList)) {
            auto entryPath = SyncStateEntry::makePath(rootDir, entry.absoluteFilePath());
            handledEntries.insert(entryPath);
            if (!filter(entryPath, FileInfo::fromLocalFile(entry.absoluteFilePath()))) {
                continue;
            }
            if (previousEntriesMap.contains(entryPath)) {
                if (entry.isDir()) {
                    paths.enqueue(entryPath); // We need to go into sub-folders to find out if
                                              // something changed. Enqueue the path.
                } else {
                    QFileInfo fi(entry.absoluteFilePath());
                    auto previousEntry = previousEntriesMap.value(entryPath);
                    if (fi.lastModified() != previousEntry.modificationTime()) {
                        // File has been updated locally. Add to change tree:
                        auto node = result.findNode(entryPath, ChangeTree::FindAndCreate);
                        node->type = ChangeTree::File;
                        node->change = ChangeTree::Changed;
                        node->lastModified = fi.lastModified();
                        node->syncAttribute = previousEntry.syncProperty();
                    }
                }
            } else {
                // The entry is new.
                auto node = result.findNode(entryPath, ChangeTree::FindAndCreate);
                if (entry.isDir()) {
                    node->type = ChangeTree::Folder;
                    paths.enqueue(entryPath);
                } else {
                    node->type = ChangeTree::File;
                }
                node->change = ChangeTree::Created;
                QFileInfo fi(entry.absoluteFilePath());
                node->lastModified = fi.lastModified();
            }
        }

        // Check if we have entries from the last run that were not found locally. This means, these
        // are deleted, so we have to add them to the change tree:
        for (const auto& previousEntry : qAsConst(previousEntries)) {
            if (!handledEntries.contains(previousEntry.path())) {
                syncStateDatabase->iterate(
                        [&](const SyncStateEntry& entry) {
                            auto node = result.findNode(entry.path(), ChangeTree::FindAndCreate);
                            node->change = ChangeTree::Deleted;
                            node->lastModified = entry.modificationTime();
                            node->syncAttribute = entry.syncProperty();
                        },
                        previousEntry.path());
            }
        }
    }
    return result;
}

void DirectorySynchronizerPrivate::buildRemoteChangeTree()
{
    switch (jobFactory->remoteChangeDetectionMode()) {
    case RemoteChangeDetectionMode::FoldersWithSyncAttributes:
        buildRemoteChangeTreeWebDAVLike();
        break;
    case RemoteChangeDetectionMode::RootFolderSyncStream:
        buildRemoteChangeTreeDropboxLike();
        break;
    }
}

void DirectorySynchronizerPrivate::buildRemoteChangeTreeWebDAVLike()
{
    while (!remoteFoldersToScan.isEmpty() && error == SynchronizerError::NoError
           && runningJobs < maxJobs) {
        auto nextRemoteFolder = remoteFoldersToScan.dequeue();
        qCDebug(log) << "Scanning" << nextRemoteFolder << "for changes";
        auto job = jobFactory->listFiles(this);
        job->setPath(remoteDirectoryPath + "/" + nextRemoteFolder);
        ++runningJobs;
        setupDefaultJobSignals(job);
        connect(job, &AbstractJob::finished, this, [=]() {
            --runningJobs;
            switch (job->error()) {
            case JobError::NoError: {
                auto previousEntry = syncStateDatabase->getEntry(nextRemoteFolder);
                qCDebug(log) << "Sync attribute of" << nextRemoteFolder << "is now"
                             << job->folder().syncAttribute() << "- previously was"
                             << previousEntry.syncProperty();
                if (job->folder().syncAttribute() != previousEntry.syncProperty()
                    || job->folder().syncAttribute().isEmpty()
                    || jobFactory->alwaysCheckSubfolders()) {
                    qCDebug(log) << "Change in " << nextRemoteFolder << "detected!";
                    auto node =
                            remoteChangeTree.findNode(nextRemoteFolder, ChangeTree::FindAndCreate);
                    node->type = ChangeTree::Folder;
                    if (previousEntry.syncProperty().isEmpty()) {
                        // Entry is new
                        node->change = ChangeTree::Created;
                    } else {
                        node->change = ChangeTree::Changed;
                    }
                    node->syncAttribute = job->folder().syncAttribute();
                    auto previousEntries = syncStateDatabase->findEntries(nextRemoteFolder);
                    auto previousEntriesMap = syncStateListToMap(previousEntries);
                    QSet<QString> handledEntries;

                    auto jobEntries = job->entries();
                    for (const auto& remoteEntry : qAsConst(jobEntries)) {
                        qCDebug(log) << "Checking remote entry" << remoteEntry.name()
                                     << "with sync attribute" << remoteEntry.syncAttribute();
                        auto remoteEntryPath = SyncStateEntry::makePath(nextRemoteFolder + "/"
                                                                        + remoteEntry.name());
                        handledEntries.insert(remoteEntryPath);
                        if (!filter(remoteEntryPath, remoteEntry)) {
                            qCDebug(log) << "Remote entry" << remoteEntry.name()
                                         << "not matched by filter - ignoring it";
                            continue;
                        }

                        auto previousRemoteEntry = previousEntriesMap.value(remoteEntryPath);
                        if (previousRemoteEntry.syncProperty() != remoteEntry.syncAttribute()
                            || previousRemoteEntry.syncProperty().isEmpty()
                            || (remoteEntry.isDirectory() && jobFactory->alwaysCheckSubfolders())) {
                            // The item changed
                            qCDebug(log) << "Change in" << remoteEntry.name() << "detected!";
                            node = remoteChangeTree.findNode(remoteEntryPath,
                                                             ChangeTree::FindAndCreate);
                            if (remoteEntry.isDirectory()) {
                                node->type = ChangeTree::Folder;
                                remoteFoldersToScan.enqueue(remoteEntryPath);
                            } else {
                                node->type = ChangeTree::File;
                            }
                            if (previousRemoteEntry.syncProperty().isEmpty()) {
                                node->change = ChangeTree::Created;
                            } else {
                                node->change = ChangeTree::Changed;
                            }
                            node->syncAttribute = remoteEntry.syncAttribute();
                        }
                    }

                    // Check if there are entries from the previous run, that we don't find locally.
                    // If this is the case, they have been deleted remotely:
                    for (const auto& previousRemoteEntry : qAsConst(previousEntries)) {
                        if (!handledEntries.contains(previousRemoteEntry.path())) {
                            syncStateDatabase->iterate(
                                    [&](const SyncStateEntry& e) {
                                        auto node = remoteChangeTree.findNode(
                                                e.path(), ChangeTree::FindAndCreate);
                                        node->change = ChangeTree::Deleted;
                                    },
                                    previousRemoteEntry.path());
                        }
                    }
                }
                break;
            }
            default:
                setError(
                        SynchronizerError::FailedListingRemoteFolder,
                        tr("Failed to list contents of the remote folder %1").arg(nextRemoteFolder),
                        job->error());
                return;
            }
            buildRemoteChangeTreeWebDAVLike(); // Continue processing remote folders
        });
        job->start();
    }

    if (error == SynchronizerError::NoError && remoteFoldersToScan.isEmpty() && runningJobs <= 0) {
        mergeChangeTrees();
    }
}

void DirectorySynchronizerPrivate::buildRemoteChangeTreeDropboxLike()
{
    // There is only one folder in the queue - the root folder.
    remoteFoldersToScan.clear();

    auto job = jobFactory->listFiles(this);
    job->setPath(remoteDirectoryPath);
    job->setRecursive(true);

    // Check if we have a sync token - we save it as sync attribute of the root folder:
    auto rootFolderEntry = syncStateDatabase->getEntry("/");
    if (rootFolderEntry.isValid() && !rootFolderEntry.syncProperty().isEmpty()) {
        // The following is for testing, but could potentially be used to work around issues when
        // retrieving "delta updates": Check if the user asked up explicitly to not use incremental
        // updates ->
        if (qEnvironmentVariableIsSet(
                    "SYNQCLIENT_DIRECTORYSYNCHRONIZER_NO_INCREMENTAL_REMOTE_FOLDER_SCAN")) {
            qCWarning(log) << "SYNQCLIENT_DIRECTORYSYNCHRONIZER_NO_INCREMENTAL_REMOTE_FOLDER_SCAN "
                              "is set - NOT using the stored cursor but instead recursively "
                              "reading the full remote folder.";
        } else {
            job->setCursor(rootFolderEntry.syncProperty());
        }
    }

    job->start();
    connect(job, &ListFilesJob::finished, this, [=]() {
        job->deleteLater();
        switch (job->error()) {
        case JobError::NoError: {
            const auto& entries = job->entries();
            QDir remoteDir(remoteDirectoryPath);
            for (const auto& entry : entries) {
                const auto entryPath = entry.path();
                if (!filter(entryPath, entry)) {
                    continue;
                }
                if (entry.isFile()) {
                    auto lastSyncStateEntry = syncStateDatabase->getEntry(entry.path());
                    if (!lastSyncStateEntry.isValid()
                        || lastSyncStateEntry.syncProperty() != entry.syncAttribute()) {
                        auto node =
                                remoteChangeTree.findNode(entry.path(), ChangeTree::FindAndCreate);
                        node->change = ChangeTree::Created;
                        node->type = ChangeTree::File;
                        node->syncAttribute = entry.syncAttribute();
                        // Check if this is a known entry - i.e. we have a change instead of a
                        // create:
                        if (lastSyncStateEntry.isValid()
                            && !lastSyncStateEntry.syncProperty().isEmpty()) {
                            node->change = ChangeTree::Changed;
                        }
                    }
                } else if (entry.isDeleted()) {
                    auto node = remoteChangeTree.findNode(entry.path(), ChangeTree::FindAndCreate);
                    node->change = ChangeTree::Deleted;
                } else {
                    // Check if we already have that folder locally. Otherwise, we would get an
                    // "impossible" local changed, remote created merge conflict, which actually
                    // would not hurt but generate useless log messages.
                    QFileInfo fi(localDirectoryPath + "/" + entry.path());
                    if (!fi.exists()) {
                        auto node =
                                remoteChangeTree.findNode(entry.path(), ChangeTree::FindAndCreate);
                        node->change = ChangeTree::Created;
                        node->type = ChangeTree::Folder;
                    }
                }
            }

            // If the listing was non-incremental (i.e. we have a full listing) we need to manually
            // check for deletions as they won't be reported in that case.
            if (!job->incremental()) {
                QSet<QString> allRemoteEntries;
                for (const auto& entry : entries) {
                    allRemoteEntries.insert(SyncStateEntry::makePath(entry.path()));
                }
                qWarning() << "All remote entries:" << allRemoteEntries;
                syncStateDatabase->iterate([&](const SyncStateEntry& dbEntry) {
                    if (dbEntry.path() == "/") {
                        // Do not consider the root folder - might not be included in remote
                        // listings.
                        return;
                    }
                    if (!allRemoteEntries.contains(dbEntry.path())) {
                        qWarning() << "Marking" << dbEntry.path() << "as deleted";
                        // The entry could not be found in the DB, so assume it has been deleted on
                        // the server side. Hence, we're going to delete it:
                        auto node = remoteChangeTree.findNode(dbEntry.path(),
                                                              ChangeTree::FindAndCreate);
                        node->change = ChangeTree::Deleted;
                        node->syncAttribute = dbEntry.syncProperty();
                    } else {
                        qWarning() << "Entry" << dbEntry.path() << "found remotely - all fine";
                    }
                });
            }

            // Save the curstor as sync attribute of the remote root folder for the sync.
            remoteFoldersSyncAttributes["/"] = job->cursor();
            mergeChangeTrees();
            break;
        }
        default:
            setError(SynchronizerError::FailedListingRemoteFolder,
                     tr("Failed to list contents of the remote folder: %1").arg(job->errorString()),
                     job->error());
            break;
        }
    });
}

/**
 * @brief Merge the local and remote change trees and derive actions to run.
 */
void DirectorySynchronizerPrivate::mergeChangeTrees()
{
    QQueue<QString> paths;
    localChangeTree.dump("Local Change Tree");
    remoteChangeTree.dump("Remote Change Tree");

    // Normalize change trees:
    localChangeTree.normalize();
    remoteChangeTree.normalize();

    localChangeTree.dump("Local Change Tree (Normalized)");
    remoteChangeTree.dump("Remote Change Tree (Normalized)");

    {
        auto topLevelChanges =
                ChangeTree::mergeNames(localChangeTree.root, remoteChangeTree.root, "/");
        for (const auto& path : qAsConst(topLevelChanges)) {
            paths.enqueue(path);
        }
    }

    while (!paths.isEmpty() && error == SynchronizerError::NoError) {
        auto path = paths.dequeue();
        auto localNode = localChangeTree.findNode(path);
        auto remoteNode = remoteChangeTree.findNode(path);
        mergeChangeNodes(path, localNode, remoteNode);
        auto childPaths = ChangeTree::mergeNames(localNode, remoteNode,
                                                 path + (path.endsWith("/") ? "" : "/"));
        for (const auto& childPath : qAsConst(childPaths)) {
            paths.enqueue(childPath);
        }
    }

    numTotalSyncActionsToRun = syncActionsToRun.length();
    updateProgress();

    if (error == SynchronizerError::NoError) {
        executeSyncPlan();
    }
}

void DirectorySynchronizerPrivate::mergeChangeNodes(const QString& path,
                                                    const ChangeTreeNode* localChange,
                                                    const ChangeTreeNode* remoteChange)
{
    ChangeTreeNode local;
    if (localChange != nullptr) {
        local = *localChange;
    }
    ChangeTreeNode remote;
    if (remoteChange != nullptr) {
        remote = *remoteChange;
    }

    switch (syncConflictStrategy) {
    case SyncConflictStrategy::LocalWins:
        mergeChangeNodesLocalWins(path, local, remote);
        break;
    case SyncConflictStrategy::RemoteWins:
        mergeChangeNodesRemoteWins(path, local, remote);
        break;
    }
}

void DirectorySynchronizerPrivate::mergeChangeNodesLocalWins(const QString& path,
                                                             const ChangeTreeNode& localChange,
                                                             const ChangeTreeNode& remoteChange)
{
    Q_Q(DirectorySynchronizer);

    switch (localChange.change) {
    case ChangeTree::Unknown:
        switch (remoteChange.change) {
        case ChangeTree::Unknown:
            // Nothing to do
            break;
        case ChangeTree::Created:
            if (remoteChange.type == ChangeTree::Folder) {
                addSyncAction(new MkDirLocalSyncAction(path, remoteChange.syncAttribute));
            } else if (remoteChange.type == ChangeTree::File) {
                addSyncAction(
                        new DownloadSyncAction(path, SyncStateEntry(), remoteChange.syncAttribute));
            }
            break;
        case ChangeTree::Changed:
            if (remoteChange.type == ChangeTree::Folder) {
                // Ensure local folder exists
                addSyncAction(new MkDirLocalSyncAction(path, remoteChange.syncAttribute));
            } else {
                addSyncAction(new DownloadSyncAction(path, syncStateDatabase->getEntry(path),
                                                     remoteChange.syncAttribute));
            }
            break;
        case ChangeTree::Deleted:
            if (localChange.type == ChangeTree::Folder
                && localChangeTree.hasAnyChange(localChange)) {
                // The (local) node is a folder and contains (recursively) some changes.
                // Hence, do not delete locally. Instead, re-create the remote folder to ensure we
                // can upload the new files.
                addSyncAction(new MkDirRemoteSyncAction(path));
            } else {
                addSyncAction(new DeleteLocalSyncAction(path, syncStateDatabase->getEntry(path)));
            }
            break;
        }
        break;
    case ChangeTree::Created:
        switch (remoteChange.change) {
        case ChangeTree::Unknown:
            if (localChange.type == ChangeTree::File) {
                addSyncAction(
                        new UploadSyncAction(path, SyncStateEntry(), localChange.lastModified));
            } else if (localChange.type == ChangeTree::Folder) {
                addSyncAction(new MkDirRemoteSyncAction(path));
            }
            break;
        case ChangeTree::Created:
            // Both created, but local wins.
            if (localChange.type == ChangeTree::File) {
                if (remoteChange.type == ChangeTree::Folder) {
                    // Remotely, we created a folder. Try to delete it.
                    addSyncAction(new DeleteRemoteSyncAction(path, SyncStateEntry()));
                }
                addSyncAction(
                        new UploadSyncAction(path, SyncStateEntry(), localChange.lastModified));
            } else if (localChange.type == ChangeTree::Folder) {
                if (remoteChange.type == ChangeTree::File) {
                    // Locally, we created a file with the same name as the folder. Try to delete:
                    addSyncAction(new DeleteRemoteSyncAction(path, SyncStateEntry()));
                }
                addSyncAction(new MkDirRemoteSyncAction(path));
            }
            break;
        case ChangeTree::Changed:
            if (localChange.type == ChangeTree::Folder && remoteChange.type == ChangeTree::Folder) {
                // This is "okay" - happens when using Dropbox style remote change discovery, as we
                // always detect local folders as being new.
            } else {
                // Note: Actually, we shouldn't land here.
                emit q->logMessageAvailable(
                        SynchronizerLogEntryType::Warning,
                        tr("Impossible sync conflict on %1: Local created, remote changed")
                                .arg(path));
                qCWarning(log) << "Impossible sync conflict on path" << path
                               << " - local created, remote changed";
            }
            break;
        case ChangeTree::Deleted:
            // Note: Actually, we shouldn't land here.
            emit q->logMessageAvailable(
                    SynchronizerLogEntryType::Warning,
                    tr("Impossible sync conflict on %1: Local created, remote deleted").arg(path));
            qCWarning(log) << "Impossible sync conflict on path" << path
                           << " - local created, remote deleted";
            break;
        }
        break;
    case ChangeTree::Changed:
        switch (remoteChange.change) {
        case ChangeTree::Unknown:
            if (localChange.type == ChangeTree::File) {
                addSyncAction(new UploadSyncAction(path, syncStateDatabase->getEntry(path),
                                                   localChange.lastModified));
            }
            break;
        case ChangeTree::Created:
            emit q->logMessageAvailable(
                    SynchronizerLogEntryType::Warning,
                    tr("Impossible sync conflict on %1: Local changed, remote created").arg(path));
            qCWarning(log) << "Impossible sync conflict on path" << path
                           << " - local changed, remote created";
            break;
        case ChangeTree::Changed:
            if (localChange.type == ChangeTree::File) {
                addSyncAction(new UploadSyncAction(path, syncStateDatabase->getEntry(path),
                                                   remoteChange.lastModified));
            }
            break;
        case ChangeTree::Deleted:
            if (localChange.type == ChangeTree::File) {
                // The remote deleted the folder, but local wins. This can only happen if the
                // resource is a file. Hence, re-upload it.
                addSyncAction(new UploadSyncAction(path, syncStateDatabase->getEntry(path),
                                                   localChange.lastModified));
            } else {
                emit q->logMessageAvailable(
                        SynchronizerLogEntryType::Warning,
                        tr("Impossible sync conflict on %1: Local folder changed, remote deleted")
                                .arg(path));
                qCWarning(log) << "Impossible sync conflict on path" << path
                               << " - local folder changed, remote deleted";
            }
            break;
        }
        break;
    case ChangeTree::Deleted:
        switch (remoteChange.change) {
        case ChangeTree::Unknown:
            // This happens if we have some changes further down the remote sync tree. In this
            // case, we must re-create the local folder:
            if (remoteChange.type == ChangeTree::Folder) {
                if (ChangeTree::has<ChangeTree::Created>(remoteChange)) {
                    addSyncAction(new MkDirLocalSyncAction(path, remoteChange.syncAttribute));
                    break;
                }
            }
            addSyncAction(new DeleteRemoteSyncAction(path, syncStateDatabase->getEntry(path)));
            break;
        case ChangeTree::Created:
            emit q->logMessageAvailable(
                    SynchronizerLogEntryType::Warning,
                    tr("Impossible sync conflict on %1: Local deleted, remote created").arg(path));
            qCWarning(log) << "Impossible sync conflict on path" << path
                           << " - local deleted, remote created";
            break;
        case ChangeTree::Changed:
            // Local wins. Nevertheless, check if this is a folder and - if so - if it contains some
            // new resources. In this case, we have to re-create locally.
            if (remoteChange.type == ChangeTree::Folder) {
                if (ChangeTree::has<ChangeTree::Created>(remoteChange)) {
                    addSyncAction(new MkDirLocalSyncAction(path, remoteChange.syncAttribute));
                    break;
                }
            }
            addSyncAction(new DeleteRemoteSyncAction(path, syncStateDatabase->getEntry(path)));
            break;
        case ChangeTree::Deleted:
            // Both deleted - fine, nothing to do!
            break;
        }
        break;
    }
}

void DirectorySynchronizerPrivate::mergeChangeNodesRemoteWins(const QString& path,
                                                              const ChangeTreeNode& localChange,
                                                              const ChangeTreeNode& remoteChange)
{
    Q_Q(DirectorySynchronizer);
    switch (localChange.change) {
    case ChangeTree::Unknown:
        switch (remoteChange.change) {
        case ChangeTree::Unknown:
            // Nothing to do
            break;
        case ChangeTree::Created:
            if (remoteChange.type == ChangeTree::Folder) {
                addSyncAction(new MkDirLocalSyncAction(path, remoteChange.syncAttribute));
            } else if (remoteChange.type == ChangeTree::File) {
                addSyncAction(
                        new DownloadSyncAction(path, SyncStateEntry(), remoteChange.syncAttribute));
            }
            break;
        case ChangeTree::Changed:
            if (remoteChange.type == ChangeTree::Folder) {
                // Ensure local folder exists
                addSyncAction(new MkDirLocalSyncAction(path, remoteChange.syncAttribute));
            } else {
                addSyncAction(new DownloadSyncAction(path, syncStateDatabase->getEntry(path),
                                                     remoteChange.syncAttribute));
            }
            break;
        case ChangeTree::Deleted:
            if (localChange.type == ChangeTree::Folder
                && localChangeTree.has<ChangeTree::Created>(localChange)) {
                // The (local) node is a folder and contains (recursively) some created node.
                // Hence, do not delete locally. Instead, re-create the remote folder to ensure we
                // can upload the new files.
                addSyncAction(new MkDirRemoteSyncAction(path));
            } else {
                addSyncAction(new DeleteLocalSyncAction(path, syncStateDatabase->getEntry(path)));
            }
            break;
        }
        break;
    case ChangeTree::Created:
        switch (remoteChange.change) {
        case ChangeTree::Unknown:
            if (localChange.type == ChangeTree::File) {
                addSyncAction(
                        new UploadSyncAction(path, SyncStateEntry(), localChange.lastModified));
            } else if (localChange.type == ChangeTree::Folder) {
                addSyncAction(new MkDirRemoteSyncAction(path));
            }
            break;
        case ChangeTree::Created:
            // Both created, but remote wins.
            if (remoteChange.type == ChangeTree::File) {
                if (localChange.type == ChangeTree::Folder) {
                    // Locally, we created a folder. Try to delete it.
                    addSyncAction(new DeleteLocalSyncAction(path, SyncStateEntry()));
                }
                addSyncAction(
                        new DownloadSyncAction(path, SyncStateEntry(), remoteChange.syncAttribute));
            } else if (remoteChange.type == ChangeTree::Folder) {
                if (localChange.type == ChangeTree::File) {
                    // Locally, we created a file with the same name as the folder. Try to delete:
                    addSyncAction(new DeleteLocalSyncAction(path, SyncStateEntry()));
                }
                addSyncAction(new MkDirLocalSyncAction(path, remoteChange.syncAttribute));
            }
            break;
        case ChangeTree::Changed:
            if (localChange.type == ChangeTree::Folder && remoteChange.type == ChangeTree::Folder) {
                // Okay, happens with Dropbox style remote change discovery
            } else {
                // Note: Actually, we shouldn't land here.
                emit q->logMessageAvailable(
                        SynchronizerLogEntryType::Warning,
                        tr("Impossible sync conflict on %1: Local created, remote changed")
                                .arg(path));
                qCWarning(log) << "Impossible sync conflict on path" << path
                               << " - local created, remote changed";
            }
            break;
        case ChangeTree::Deleted:
            // Note: Actually, we shouldn't land here.
            emit q->logMessageAvailable(
                    SynchronizerLogEntryType::Warning,
                    tr("Impossible sync conflict on %1: Local created, remote deleted").arg(path));
            qCWarning(log) << "Impossible sync conflict on path" << path
                           << " - local created, remote deleted";
            break;
        }
        break;
    case ChangeTree::Changed:
        switch (remoteChange.change) {
        case ChangeTree::Unknown:
            if (localChange.type == ChangeTree::File) {
                addSyncAction(new UploadSyncAction(path, syncStateDatabase->getEntry(path),
                                                   localChange.lastModified));
            }
            break;
        case ChangeTree::Created:
            emit q->logMessageAvailable(
                    SynchronizerLogEntryType::Warning,
                    tr("Impossible sync conflict on %1: Local changed, remote created").arg(path));
            qCWarning(log) << "Impossible sync conflict on path" << path
                           << " - local changed, remote created";
            break;
        case ChangeTree::Changed:
            if (remoteChange.type == ChangeTree::File) {
                addSyncAction(new DownloadSyncAction(path, syncStateDatabase->getEntry(path),
                                                     remoteChange.syncAttribute));
            }
            break;
        case ChangeTree::Deleted:
            if (localChange.type == ChangeTree::Folder) {
                // The remote deleted the folder, which wins. However, check if we have new files.
                // In this case, we re-create the remote folder.
                if (ChangeTree::has<ChangeTree::Created>(localChange)) {
                    addSyncAction(new MkDirRemoteSyncAction(path));
                    break;
                }
            }
            addSyncAction(new DeleteLocalSyncAction(path, syncStateDatabase->getEntry(path)));
            break;
        }
        break;
    case ChangeTree::Deleted:
        switch (remoteChange.change) {
        case ChangeTree::Unknown:
            // This happens if we have some changes further down the local sync tree. In this
            // case, we must re-create the local folder:
            if (remoteChange.type == ChangeTree::Folder) {
                if (ChangeTree::has<ChangeTree::Created>(localChange)) {
                    addSyncAction(new MkDirLocalSyncAction(path, remoteChange.syncAttribute));
                    break;
                }
            }
            addSyncAction(new DeleteRemoteSyncAction(path, syncStateDatabase->getEntry(path)));
            break;
        case ChangeTree::Created:
            emit q->logMessageAvailable(
                    SynchronizerLogEntryType::Warning,
                    tr("Impossible sync conflict on %1: Local deleted, remote created").arg(path));
            qCWarning(log) << "Impossible sync conflict on path" << path
                           << " - local deleted, remote created";
            break;
        case ChangeTree::Changed:
            // Remote wins. Check if the remote is a folder and recreate it locally:
            if (remoteChange.type == ChangeTree::Folder) {
                addSyncAction(new MkDirLocalSyncAction(path, remoteChange.syncAttribute));
            } else if (remoteChange.type == ChangeTree::File) {
                addSyncAction(new DownloadSyncAction(path, syncStateDatabase->getEntry(path),
                                                     remoteChange.syncAttribute));
            }
            break;
        case ChangeTree::Deleted:
            // Both deleted - fine, nothing to do!
            break;
        }
        break;
    }
}

void DirectorySynchronizerPrivate::addSyncAction(SyncAction* action)
{
    syncActionsToRun << QSharedPointer<SyncAction>(action);
}

/**
 * @brief Run any local actions that do not require server interaction.
 */
void DirectorySynchronizerPrivate::runLocalActions()
{
    Q_Q(DirectorySynchronizer);
    decltype(syncActionsToRun) remainingSyncActions;
    for (const auto& action : qAsConst(syncActionsToRun)) {
        switch (action->type) {
        case MkDirLocal: {
            qCDebug(log) << "Creating local folder" << action->path;
            emit q->logMessageAvailable(SynchronizerLogEntryType::LocalMkDir, action->path);
            QDir dir(localDirectoryPath + "/" + action->path);
            if (!dir.mkpath(".")) {
                setError(SynchronizerError::FailedCreatingLocalFolder,
                         tr("Creating folder %1 has failed").arg(dir.absolutePath()),
                         JobError::NoError);
                return;
            }
            remoteFoldersSyncAttributes[action->path] =
                    qSharedPointerCast<MkDirLocalSyncAction>(action)->syncAttribute;
            break;
        }
        case DeleteLocal:
            qCDebug(log) << "Deleting local resource" << action->path;
            emit q->logMessageAvailable(SynchronizerLogEntryType::LocalDelete, action->path);

            if (!deleteLocally(action->path)) {
                return;
            } else {
                if (!syncStateDatabase->removeEntries(action->path)
                    || !syncStateDatabase->removeEntry(action->path)) {
                    setError(SynchronizerError::SyncStateDatabaseDeleteFailed,
                             tr("Failed to delete entries from the sync state database"),
                             JobError::NoError);
                    return;
                }
            }
            break;
        case Upload:
        case Download:
        case MkDirRemote:
        case DeleteRemote:
            // Nothing to do for now
            remainingSyncActions << action;
            break;
        }
    }

    syncActionsToRun = remainingSyncActions;
}

void DirectorySynchronizerPrivate::runRemoteActions()
{
    if (error != SynchronizerError::NoError) {
        finishLater();
        return;
    }

    updateProgress();

    if (runningJobs >= maxJobs) {
        return;
    }

    decltype(syncActionsToRun) remainingSyncActions;
    for (const auto& action : qAsConst(syncActionsToRun)) {
        if (!canRunAction(action)) {
            remainingSyncActions << action;
            continue;
        }

        if (runningJobs >= maxJobs) {
            remainingSyncActions << action;
            continue;
        }

        runRemoteAction(action);
    }

    if (remainingSyncActions.length() == syncActionsToRun.length() && runningJobs <= 0
        && !syncActionsToRun.isEmpty()) {
        setError(SynchronizerError::Stuck, tr("Cannot continue sync - it is stuck"),
                 JobError::NoError);
        return;
    }

    syncActionsToRun = remainingSyncActions;

    if (syncActionsToRun.isEmpty() && runningJobs <= 0) {
        if (error == SynchronizerError::NoError) {
            // Safe remote folder sync attributes. This only is done if we don't have any errors.
            // This will e.g. cause us to download/upload again in case we have failed transfers.
            // Otherwise, we won't re-detect remote changes (as we only compare the parent folders'
            // sync attributes).
            for (auto it = remoteFoldersSyncAttributes.cbegin();
                 it != remoteFoldersSyncAttributes.cend(); ++it) {
                if (!syncStateDatabase->addEntry(
                            SyncStateEntry(it.key(), QDateTime(), it.value()))) {
                    setError(SynchronizerError::SyncStateDatabaseWriteFailed,
                             tr("Failed to write folder sync attribute to sync state database"),
                             JobError::NoError);
                    break;
                }
            }
        }
        finishLater();
    }
}

bool DirectorySynchronizerPrivate::deleteLocally(const QString& path)
{
    auto fullPath = QDir::cleanPath(localDirectoryPath + "/" + path);
    QFileInfo fi(fullPath);
    if (fi.isFile()) {
        QFile file(fullPath);
        if (file.exists() && !file.remove()) {
            setError(SynchronizerError::FailedDeletingLocalFile,
                     tr("Failed deleting local file %1: %2")
                             .arg(fi.absolutePath(), file.errorString()),
                     JobError::NoError);
            return false;
        }
    } else {
        QDir dir(fullPath);
        if (dir.exists()) {
            {
                QDirIterator it(fullPath, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot,
                                QDirIterator::Subdirectories);
                while (it.hasNext()) {
                    qCWarning(log) << it.next();
                }
            }
            {
                QDirIterator it(fullPath, QDir::Files, QDirIterator::Subdirectories);
                while (it.hasNext()) {
                    auto next = it.next();
                    auto entryPath =
                            SyncStateEntry::makePath("/" + path + "/" + dir.relativeFilePath(next));
                    if (filter(entryPath, FileInfo::fromLocalFile(next))) {
                        QFile file(next);
                        if (!file.remove()) {
                            setError(SynchronizerError::FailedDeletingLocalFile,
                                     tr("Failed deleting local file %1: %2")
                                             .arg(next, file.errorString()),
                                     JobError::NoError);
                            return false;
                        }
                    }
                }
            }
            {
                QDirIterator it(fullPath, QDir::Dirs | QDir::NoDotAndDotDot,
                                QDirIterator::Subdirectories);
                while (it.hasNext()) {
                    auto next = it.next();
                    auto entryPath =
                            SyncStateEntry::makePath("/" + path + "/" + dir.relativeFilePath(next));
                    if (filter(entryPath, FileInfo::fromLocalFile(next))) {
                        QFileInfo dirFi(next);
                        if (!dirFi.dir().rmdir(dirFi.fileName())) {
                            setError(SynchronizerError::FailedDeletingLocalFolder,
                                     tr("Failed deleting local folder %1").arg(next),
                                     JobError::NoError);
                            return false;
                        }
                    }
                }
            }
            QFileInfo dirFi(fullPath);
            if (!dirFi.dir().rmdir(dirFi.fileName())) {
                setError(SynchronizerError::FailedDeletingLocalFolder,
                         tr("Failed deleting local folder %1").arg(dir.absolutePath()),
                         JobError::NoError);
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief Check if the action can be run.
 *
 * Actions cannot run if one of the following is true:
 *
 * - The action's path is a prefix of a resource that still is about to be deleted remotely.
 * - A remote folder, which is a parent of the action's path still needs to be created.
 *
 * Note that - by this - in particular we first delete remote resources before re-creating them (to
 * solve type changes).
 */
bool DirectorySynchronizerPrivate::canRunAction(const QSharedPointer<SyncAction>& action)
{
    for (const auto& path : qAsConst(remoteResourcesToDelete)) {
        if (path.startsWith(action->path)) {
            // This action is potentially blocked by the path. Exception: The action will delete
            // exactly this resource:
            if (path != action->path) {
                // This action deletes another resource - we have to wait.
                return false;
            }
        }
    }

    for (const auto& path : qAsConst(remoteFoldersToCreate)) {
        // Check if the path to be created is a prefix of the action:
        if (action->path.startsWith(path + "/")) {
            if (action->type == MkDirRemote) {
                // If this is an action to create a remote folder, we have to look closer...
                if (action->path != path) {
                    // One of the parents of the folder to be created still needs to be created
                    // itself. Hence, we cannot start this one yet:
                    return false;
                } else {
                    // This path will be created by this action. However, there are still other
                    // paths in the list - hence, we cannot yet "return true".
                }
            } else {
                // For any other kind of action, that's it, we have to wait:
                return false;
            }
        }
    }

    // Still here? Than we are ready to run the action:
    return true;
}

void DirectorySynchronizerPrivate::runRemoteAction(const QSharedPointer<SyncAction>& action)
{
    Q_Q(DirectorySynchronizer);
    switch (action->type) {
    case MkDirLocal:
    case DeleteLocal:
        // There shouldn't be any such actions left
        qCWarning(log) << "We should not have any MkDirLocal and DeleteLocal actions in"
                       << __func__;
        emit q->logMessageAvailable(SynchronizerLogEntryType::Warning,
                                    tr("Found local action in remote action execution phase"));
        break;
    case Upload: {
        qCDebug(log) << "Uploading" << action->path;
        emit q->logMessageAvailable(SynchronizerLogEntryType::Upload, action->path);
        ++runningJobs;
        auto job = jobFactory->uploadFile(this);
        job->setLocalFilename(localDirectoryPath + "/" + action->path);
        job->setRemoteFilename(remoteDirectoryPath + "/" + action->path);
        auto uploadAction = qSharedPointerCast<UploadSyncAction>(action);
        if (uploadAction->previousSyncEntry.isValid()
            && syncConflictStrategy != SyncConflictStrategy::LocalWins) {
            job->setSyncAttribute(uploadAction->previousSyncEntry.syncProperty());
        }
        setupDefaultJobSignals(job);
        connect(job, &AbstractJob::finished, this, [=]() {
            --runningJobs;
            switch (job->error()) {
            case JobError::NoError:
                // Uploading succeeded. Save sync attribute
                if (!job->fileInfo().syncAttribute().isEmpty()) {
                    if (!syncStateDatabase->addEntry(
                                SyncStateEntry(uploadAction->path, uploadAction->lastModified,
                                               job->fileInfo().syncAttribute()))) {
                        setError(SynchronizerError::SyncStateDatabaseWriteFailed,
                                 tr("Failed to write to the sync state database"),
                                 JobError::NoError);
                        return;
                    }
                    runRemoteActions();
                } else {
                    // We did not receive a sync attribute on upload - fetch one from the server.
                    qCDebug(log) << "Didn't get a sync attribute on upload - fetching from server";
                    auto fileInfoJob = jobFactory->getFileInfo();
                    fileInfoJob->setPath(job->remoteFilename());
                    setupDefaultJobSignals(fileInfoJob);
                    fileInfoJob->start();
                    connect(fileInfoJob, &AbstractJob::finished, this, [=]() {
                        fileInfoJob->deleteLater();
                        if (fileInfoJob->error() == JobError::NoError) {
                            QString syncAttribute;
                            syncAttribute = fileInfoJob->fileInfo().syncAttribute();
                            qCDebug(log) << "Manually fetched sync attribute for"
                                         << fileInfoJob->path() << "from server:" << syncAttribute;
                            if (!syncStateDatabase->addEntry(SyncStateEntry(
                                        uploadAction->path, uploadAction->lastModified,
                                        syncAttribute))) {
                                setError(SynchronizerError::SyncStateDatabaseWriteFailed,
                                         tr("Failed to write to the sync state database"),
                                         JobError::NoError);
                                return;
                            }
                            runRemoteActions();
                            return;
                        } else {
                            setError(SynchronizerError::UploadFailed,
                                     tr("Failed to fetch file info from remote server: %1")
                                             .arg(fileInfoJob->errorString()),
                                     fileInfoJob->error());
                        }
                    });
                    return;
                }
                break;
            case JobError::SyncAttributeMismatch:
                // There was a lost update (i.e. another client uploaded meanwhile).
                break;
            default:
                setError(SynchronizerError::UploadFailed,
                         tr("Uploading %1 failed: %2").arg(uploadAction->path, job->errorString()),
                         job->error());
                return;
            }
            runRemoteActions();
        });
        job->start();
        break;
    }

    case Download: {
        qCDebug(log) << "Downloading" << action->path;
        emit q->logMessageAvailable(SynchronizerLogEntryType::Download, action->path);
        ++runningJobs;
        auto job = jobFactory->downloadFile(this);
        job->setRemoteFilename(remoteDirectoryPath + "/" + action->path);
        auto saveFile = new QSaveFile(job);
        saveFile->setFileName(localDirectoryPath + "/" + action->path);
        auto parentDir = QFileInfo(saveFile->fileName()).dir();
        if (!parentDir.exists() && !parentDir.mkpath(".")) {
            setError(SynchronizerError::FailedCreatingLocalFolder,
                     tr("Failed to create the local folder %1").arg(parentDir.path()),
                     JobError::NoError);
            delete job;
            return;
        }
        if (!saveFile->open(QIODevice::WriteOnly)) {
            setError(SynchronizerError::OpeningLocalFileFailed,
                     tr("Opening file %1 for reading/writing failed: %2")
                             .arg(saveFile->fileName(), saveFile->errorString()),
                     JobError::NoError);
            delete job;
            return;
        }
        job->setOutput(saveFile);
        setupDefaultJobSignals(job);
        QSharedPointer<DownloadSyncAction> downloadAction =
                qSharedPointerCast<DownloadSyncAction>(action);
        connect(job, &AbstractJob::finished, this, [=]() {
            --runningJobs;
            switch (job->error()) {
            case JobError::NoError:
                // Download succeeded. Now check if the time stamp still matches.
                if (syncConflictStrategy == SyncConflictStrategy::LocalWins
                    && downloadAction->previousSyncEntry.isValid()) {
                    QFileInfo fi(saveFile->fileName());
                    if (fi.lastModified() > downloadAction->previousSyncEntry.modificationTime()) {
                        // Lost update
                        runRemoteActions();
                        return;
                    }
                }
                // Commit to DB
                if (saveFile->commit()) {
                    auto syncAttribute = job->fileInfo().syncAttribute();
                    if (syncAttribute.isNull()) {
                        // Use the sync attribute from the list files job. This might
                        // already be outdated - in this case, we will download again on
                        // the next sync.
                        syncAttribute = downloadAction->syncAttribute;
                    }
                    if (!syncStateDatabase->addEntry(SyncStateEntry(
                                downloadAction->path,
                                QFileInfo(saveFile->fileName()).lastModified(), syncAttribute))) {
                        setError(SynchronizerError::SyncStateDatabaseWriteFailed,
                                 tr("Failed to write to sync state database"), JobError::NoError);
                        return;
                    }
                } else {
                    setError(SynchronizerError::WritingToLocalFileFailed,
                             tr("Failed to commit downloaded data to file %1: %2")
                                     .arg(saveFile->fileName(), saveFile->errorString()),
                             JobError::NoError);
                    return;
                }
                break;
            default:
                setError(SynchronizerError::DownloadFailed,
                         tr("Downloading %1 failed: %2")
                                 .arg(downloadAction->path, job->errorString()),
                         job->error());
                return;
            }
            runRemoteActions();
        });
        job->start();
        break;
    }

    case DeleteRemote: {
        qCDebug(log) << "Deleting remote" << action->path;
        emit q->logMessageAvailable(SynchronizerLogEntryType::RemoteDelete, action->path);
        // Deletions are tricky. The problem: We don't want to unconditionally remove any remote
        // resource. In case of folders, other clients might update in between. However, for files,
        // we want to avoid accidentally deleting them. So what we do: The sync algorithm will
        // generate individual delete calls for each element in a hierarchy to be deleted. Before
        // actually deleting, we query the current state of the remote node. Only it it has no
        // children, we will try to delete it. Additionally, if it is a folder, we omit the
        // sync-property (as this does not work at least against WebDAV) for some reasons.

        // TODO: Check why a delete with if-match when using WebDAV fails against a folder.
        ++runningJobs;
        auto listJob = jobFactory->listFiles(this);
        listJob->setPath(remoteDirectoryPath + "/" + action->path);
        setupDefaultJobSignals(listJob);
        connect(listJob, &AbstractJob::finished, this, [=]() {
            switch (listJob->error()) {
            case JobError::NoError: {
                if (!listJob->entries().isEmpty()) {
                    QStringList remoteEntryNames;
                    auto listJobEntries = listJob->entries();
                    for (const auto& remoteEntry : qAsConst(listJobEntries)) {
                        remoteEntryNames << remoteEntry.name();
                    }
                    setError(SynchronizerError::FailedDeletingRemoteResource,
                             tr("Remote resource %1 is not empty (it still contains %2)")
                                     .arg(action->path, remoteEntryNames.join(", ")),
                             JobError::NoError);
                    return;
                }
                auto job = jobFactory->deleteResource(this);
                job->setPath(remoteDirectoryPath + "/" + action->path);
                // Does not work, see comment above.
                // job->setSyncAttribute(listJob->folder().syncAttribute());
                setupDefaultJobSignals(job);
                connect(job, &AbstractJob::finished, this, [=]() {
                    --runningJobs;
                    switch (job->error()) {
                    case JobError::NoError:
                    case JobError::ResourceNotFound:
                        syncStateDatabase->removeEntry(action->path);
                        syncStateDatabase->removeEntries(action->path);
                        remoteResourcesToDelete.removeAll(action->path);
                        break;
                    case JobError::SyncAttributeMismatch:
                        // The resource was updated meanwhile. This could be because we are
                        // recursively deleting. In this case, re-run this action:
                        if (action->retries < 5 && error == SynchronizerError::NoError) {
                            action->retries += 1;
                            runRemoteAction(action);
                            break;
                        } else {
                            setError(SynchronizerError::FailedDeletingRemoteResource,
                                     tr("Permanently failed to remove remote resource %1 after %2 "
                                        "retries - it keeps being updated too fast | %3")
                                             .arg(action->path)
                                             .arg(action->retries)
                                             .arg(job->syncAttribute().toString()),
                                     job->error());
                            return;
                        }
                    default:
                        setError(SynchronizerError::FailedDeletingRemoteResource,
                                 tr("Failed deleting remote resource %1: %2")
                                         .arg(action->path, job->errorString()),
                                 job->error());
                        return;
                    }
                    runRemoteActions(); // Currently dead code - keep to ensure we "keep going" in
                                        // case of future changes.
                });
                job->start();
                break;
            }
            case JobError::ResourceNotFound:
                // The resource is no longer present - fine!
                --runningJobs;
                remoteResourcesToDelete.removeAll(action->path);
                runRemoteActions();
                break;
            default:
                setError(SynchronizerError::FailedDeletingRemoteResource,
                         tr("Failed to list remote resource %1: %2")
                                 .arg(action->path, listJob->errorString()),
                         listJob->error());
                break;
            }
        });
        listJob->start();
        break;
    }

    case MkDirRemote: {
        qCDebug(log) << "Creating remote folder" << action->path;
        emit q->logMessageAvailable(SynchronizerLogEntryType::RemoteMkDir, action->path);
        ++runningJobs;
        auto job = jobFactory->createDirectory(this);
        job->setPath(remoteDirectoryPath + "/" + action->path);
        setupDefaultJobSignals(job);
        connect(job, &AbstractJob::finished, this, [=]() {
            --runningJobs;
            switch (job->error()) {
            case JobError::NoError:
            case JobError::FolderExists: {
                remoteFoldersToCreate.removeAll(action->path);
                break;
            }
            default:
                setError(SynchronizerError::FailedCreatingRemoteFolder,
                         tr("Failed to create remote folder %1: %2")
                                 .arg(action->path, job->errorString()),
                         job->error());
                return;
            }
            runRemoteActions();
        });
        job->start();
        break;
    }
    }
}

void DirectorySynchronizerPrivate::updateProgress()
{
    Q_Q(DirectorySynchronizer);
    if (numTotalSyncActionsToRun > 0) {
        auto numRemaining = syncActionsToRun.length();
        progress = (numTotalSyncActionsToRun - numRemaining) * 100 / numTotalSyncActionsToRun;
    }
    emit q->progress(progress);
}

void DirectorySynchronizerPrivate::finishLater()
{
    Q_Q(DirectorySynchronizer);
    QTimer::singleShot(0, q, [=] {
        if (state == SynchronizerState::Running) {
            if (syncStateDatabase && syncStateDatabase->isOpen()
                && !syncStateDatabase->closeDatabase()) {
                if (error == SynchronizerError::NoError) {
                    setError(SynchronizerError::FailedClosingSyncStateDatabase,
                             tr("Failed to close the sync state database"), JobError::NoError);
                }
            }
            state = SynchronizerState::Finished;
            emit q->finished();
        }
    });
}

void DirectorySynchronizerPrivate::setError(SynchronizerError error, const QString& errorString,
                                            JobError jobError)
{
    Q_Q(DirectorySynchronizer);
    emit q->logMessageAvailable(SynchronizerLogEntryType::Error, errorString);
    if (this->error == SynchronizerError::NoError) {
        // Check if this could be a server overload scenario - if so, check if we should retry
        // with fewer parallel jobs:
        if (jobError == JobError::ServerClosedConnection && maxJobs > 1) {
            this->retryWithFewerJobs = true;
        }
        this->error = error;
        this->errorString = errorString;
    }
    finishLater();
}

void DirectorySynchronizerPrivate::createRemoteFolder()
{
    Q_Q(DirectorySynchronizer);
    qCDebug(log) << "Creating remote folder";
    emit q->logMessageAvailable(SynchronizerLogEntryType::Information,
                                tr("Creating remote root folder"));
    remoteFolderPartsToCreate = remoteDirectoryPath.split("/", Qt::SkipEmptyParts);
    createdRemoteFolderParts.clear();
    createNextRemoteFolderPart();
}

void DirectorySynchronizerPrivate::createSyncPlan()
{
    Q_Q(DirectorySynchronizer);
    qCDebug(log) << "Creating sync plan";
    qCDebug(log) << "Building local change tree";
    emit q->logMessageAvailable(SynchronizerLogEntryType::Information, tr("Creating sync plan"));
    localChangeTree = buildLocalChangeTree();
    if (error == SynchronizerError::NoError) {
        qCDebug(log) << "Building remote change tree";
        remoteFoldersToScan.enqueue("/");
        buildRemoteChangeTree();
    }
}

void DirectorySynchronizerPrivate::executeSyncPlan()
{
    Q_Q(DirectorySynchronizer);
    qCDebug(log) << "Executing sync plan";
    emit q->logMessageAvailable(SynchronizerLogEntryType::Information, tr("Executing sync plan"));
    if (error != SynchronizerError::NoError) {
        finishLater();
        return;
    }

    qCDebug(log) << "Running local sync actions";
    runLocalActions();

    // Populate list of remote folders to be created and resources to be deleted:
    for (const auto& action : syncActionsToRun) {
        switch (action->type) {
        case MkDirRemote:
            remoteFoldersToCreate << action->path;
            break;
        case DeleteRemote:
            remoteResourcesToDelete << action->path;
            break;
        default:
            // nothing to do
            break;
        }
    }

    updateProgress();

    if (error == SynchronizerError::NoError) {
        qCDebug(log) << "Running remote sync actions";
        runRemoteActions();
    } else {
        finishLater();
    }
}

} // namespace SynqClient
