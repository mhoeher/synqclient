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

#include "directorysynchronizerprivate.h"

#include <algorithm>

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QLoggingCategory>
#include <QSaveFile>
#include <QTimer>

#include "abstractjobfactory.h"
#include "createdirectoryjob.h"
#include "deletejob.h"
#include "downloadfilejob.h"
#include "getfileinfojob.h"
#include "listfilesjob.h"
#include "syncstatedatabase.h"
#include "uploadfilejob.h"

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
      syncConflictStrategy(SyncConflictStrategy::RemoteWins),
      flags(SynchronizerFlag::DefaultFlags),
      stopped(false),
      remoteFoldersSyncAttributes(),
      runningJobs(0),
      createdRemoteFolderParts(),
      remoteFolderPartsToCreate(),
      foldersToScan(),
      syncActionsToRun(),
      remoteFoldersToCreate()
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
                     tr("Failed creating remote directory: %1").arg(job->errorString()));
        }
    });
    job->start();
}

/**
 * @brief Start scanning folders queued for sync.
 *
 * This will start scanning folders, both remotely and locally, that have previously been queued.
 *
 * Scanning local folders os done sequentially. If folders need to be scanned remotely, up to the
 * configured number of jobs is created to try gathering remote folder information in parallel.
 *
 * Once the queue is empty and all jobs finished running, this will start the next phase of the
 * sync.
 */
void DirectorySynchronizerPrivate::scanNextFolder()
{
    if (stopped) {
        finishLater();
        return;
    }

    // Scan listed directories:
    while (!foldersToScan.isEmpty() && runningJobs < maxJobs
           && error == SynchronizerError::NoError) {
        auto nextFolder = foldersToScan.takeFirst();
        if (nextFolder.remoteState == RemoteUnchanged) {
            // We detected no changes remotely - only check if there are local changes.
            scanLocalFolderOnly(nextFolder);
        } else {
            // The remote folder changed. We need to query the current state from the remote
            // and merge both local and remote change sets to create the sync plan.
            scanLocalAndRemoteFolder(nextFolder);
        }
    }

    if (runningJobs <= 0 && error == SynchronizerError::NoError) {
        executeSyncPlan();
    }
}

/**
 * @brief Scan the local folder for changes.
 *
 * This will try to find changes in the folder only locally.
 */
void DirectorySynchronizerPrivate::scanLocalFolderOnly(
        const DirectorySynchronizerPrivate::ScanRecord& folder)
{
    bool ok;
    auto lastRunEntries = syncStateDatabase->findEntries(folder.path, &ok);
    if (!ok) {
        setError(SynchronizerError::SyncStateDatabaseLookupFailed,
                 "Reading from the sync state database failed");
        return;
    }

    auto lastRunEntriesMap = syncStateListToMap(lastRunEntries);

    QSet<QString> localSubFolders;
    auto localChanges = findLocalChanges(lastRunEntriesMap, localDirectoryPath + "/" + folder.path,
                                         ok, localSubFolders);
    resolveChanges(localChanges, QMap<QString, Change>(), localSubFolders,
                   QMap<QString, RemoteEntryState>());
}

/**
 * @brief Check a local and remote folder for changes.
 * @param folder
 */
void DirectorySynchronizerPrivate::scanLocalAndRemoteFolder(
        const DirectorySynchronizerPrivate::ScanRecord& folder)
{
    bool ok;
    auto lastRunEntries = syncStateDatabase->findEntries(folder.path, &ok);
    if (!ok) {
        setError(SynchronizerError::SyncStateDatabaseLookupFailed,
                 "Reading from the sync state database failed");
        return;
    }

    auto lastRunEntriesMap = syncStateListToMap(lastRunEntries);

    auto job = jobFactory->listFiles(this);
    job->setPath(QDir::cleanPath(remoteDirectoryPath + "/" + folder.path));
    setupDefaultJobSignals(job);
    auto thisDir = folder.path;
    connect(job, &AbstractJob::finished, this, [=]() {
        if (job->error() == JobError::NoError) {
            --runningJobs;
            bool ok2;
            QSet<QString> localSubFolders;
            QMap<QString, RemoteEntryState> remoteSubFolders;
            auto remoteChanges = findRemoteChanges(lastRunEntriesMap, job->entries(), thisDir, ok2,
                                                   remoteSubFolders);

            if (!ok2) {
                return;
            }
            auto localChanges = findLocalChanges(
                    lastRunEntriesMap, localDirectoryPath + "/" + thisDir, ok2, localSubFolders);
            if (!ok2) {
                return;
            }

            resolveChanges(localChanges, remoteChanges, localSubFolders, remoteSubFolders);
            scanNextFolder();
        } else {
            setError(SynchronizerError::FailedListingRemoteFolder,
                     tr("Failed listing entries in remote folder %1: %2")
                             .arg(thisDir, job->errorString()));
        }
    });
    ++runningJobs;
    job->start();
}

QMap<QString, DirectorySynchronizerPrivate::Change>
DirectorySynchronizerPrivate::findLocalChanges(const QMap<QString, SyncStateEntry> lastRunEntries,
                                               const QString& path, bool& ok,
                                               QSet<QString>& subFolders)
{
    QMap<QString, Change> result;
    ok = true;

    QSet<QString> handledEntries;
    QDir localRootDir(localDirectoryPath);
    QDir dir(path);
    const auto localEntries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    for (const auto& entry : localEntries) {
        QFileInfo fi(entry.absoluteFilePath());
        auto entryPath =
                SyncStateEntry::makePath(localRootDir.relativeFilePath(entry.absoluteFilePath()));
        FileInfo fileInfo;
        if (fi.isDir()) {
            fileInfo.setIsDirectory();
            subFolders.insert(entryPath);
        } else {
            fileInfo.setIsFile();
        }
        fileInfo.setName(entry.completeBaseName());
        if (filter(entryPath, fileInfo)) {
            auto currentModTime = fi.lastModified();
            if (!currentModTime.isValid()) {
                setError(SynchronizerError::FailedGettingLocalModificationTime,
                         "Failed to get the last modification time of " + fi.absoluteFilePath());
                ok = false;
                return result;
            }
            if (lastRunEntries.contains(entryPath)) {
                // We know the entry from the last run, check if there are local updates:
                if (fi.isDir()) {
                    // The entry is a local folder. As it already exists locally and there are no
                    // changes on the remote, there is nothing todo.
                } else {
                    // The entry is a file. Check if we need to upload it:
                    const auto& lastRunEntry = lastRunEntries.value(entryPath);
                    auto lastModTime = lastRunEntry.modificationTime();
                    if (currentModTime > lastModTime) {
                        result[entryPath] = { Change::ChangedLocally, currentModTime, QString() };
                    }
                }
            } else {
                // The entry is not in our database - assume it has been created locally.
                if (fi.isFile()) {
                    result[entryPath] = { Change::AddedLocally, fi.lastModified(), QString() };
                } else {
                    // Create the remote folder:
                    result[entryPath] = { Change::CreatedDirLocally, QDateTime(), QString() };
                }
            }
        }
        handledEntries.insert(entryPath);
    }

    std::for_each(lastRunEntries.cbegin(), lastRunEntries.cend(),
                  [=, &result](const SyncStateEntry& entry) {
                      if (!handledEntries.contains(entry.path())) {
                          // We did not find the file locally - this means it has been deleted
                          // locally
                          result[entry.path()] = { Change::DeletedLocally, QDateTime(), QString() };
                      }
                  });

    return result;
}

QMap<QString, DirectorySynchronizerPrivate::Change> DirectorySynchronizerPrivate::findRemoteChanges(
        const QMap<QString, SyncStateEntry> lastRunEntries, const QVector<FileInfo>& remoteEntries,
        const QString& path, bool& ok, QMap<QString, RemoteEntryState>& subFolders)
{
    QMap<QString, Change> result;
    ok = true;

    QSet<QString> handledEntries;

    for (const auto& entry : remoteEntries) {
        if (!entry.isValid()) {
            continue;
        }
        auto entryPath = SyncStateEntry::makePath(path + "/" + entry.name());
        if (!filter(entryPath, entry)) {
            continue;
        }
        if (lastRunEntries.contains(entryPath)) {
            // The entry is known - check if it changes
            auto localEntry = lastRunEntries[entryPath];
            if (localEntry.syncProperty() != entry.syncAttribute()) {
                // The entry has been changed.
                if (entry.isFile()) {
                    // It is a file - i.e. we need to download it:
                    result[entryPath] = { Change::ChangedRemotely, QDateTime(),
                                          entry.syncAttribute() };
                } else {
                    // It is a folder - we need to check it later:
                    subFolders[entryPath] = RemoteChanged;
                    remoteFoldersSyncAttributes[entryPath] = entry.syncAttribute();
                }
            } else {
                if (entry.isDirectory()) {
                    // The remote is an unchanged remote folder. Remember it, we might still
                    // have to scan it:
                    subFolders[entryPath] = RemoteUnchanged;
                }
            }
        } else {
            // The entry is not known - it has been added on the server
            if (entry.isFile()) {
                // The file needs to be downloaded
                result[entryPath] = { Change::AddedRemotely, QDateTime(), entry.syncAttribute() };
            } else if (entry.isDirectory()) {
                result[entryPath] = { Change::CreatedDirRemotely, QDateTime(),
                                      entry.syncAttribute() };
                // We need to scan the folder for contents
                subFolders[entryPath] = RemoteChanged;
            }
        }
        handledEntries.insert(entryPath);
    }

    std::for_each(
            lastRunEntries.cbegin(), lastRunEntries.cend(),
            [=, &result](const SyncStateEntry& entry) {
                if (!handledEntries.contains(entry.path())) {
                    // The entry was not found remotely - this means, it has been deleted remotely
                    result[entry.path()] = { Change::DeletedRemotely, QDateTime(), QString() };
                }
            });

    return result;
}

void DirectorySynchronizerPrivate::resolveChanges(
        const QMap<QString, DirectorySynchronizerPrivate::Change> localChanges,
        const QMap<QString, DirectorySynchronizerPrivate::Change> remoteChanges,
        const QSet<QString>& localSubFolders,
        const QMap<QString, RemoteEntryState>& remoteSubFolders)
{
    switch (syncConflictStrategy) {
    case SyncConflictStrategy::RemoteWins:
        // First process remote changes and override local ones on conflict:
        {
            for (auto it = remoteChanges.cbegin(); it != remoteChanges.cend(); ++it) {
                addActionsForChange(it.key(), it.value());
            }
            for (auto it = localChanges.cbegin(); it != localChanges.cend(); ++it) {
                const auto& path = it.key();
                const auto& change = it.value();
                if (!remoteChanges.contains(path)) {
                    // Special case: If the path is a folder and was deleted locally but has remote
                    // changes, do not create remote delete actions (but rather add local mkdir
                    // actions to recreate the structure):
                    if (change.type == Change::DeletedLocally
                        && remoteSubFolders.value(path, RemoteUnchanged) == RemoteChanged) {
                        addSyncAction(new MkDirLocalSyncAction(
                                path, remoteFoldersSyncAttributes.value(path)));
                    } else {
                        addActionsForChange(path, change);
                    }
                }
            }
            break;
        }
    case SyncConflictStrategy::LocalWins:
        // First process local changes and override remote ones on conflict:
        {
            for (auto it = localChanges.cbegin(); it != localChanges.cend(); ++it) {
                addActionsForChange(it.key(), it.value());
            }
            for (auto it = remoteChanges.cbegin(); it != remoteChanges.cend(); ++it) {
                const auto& path = it.key();
                const auto& change = it.value();
                if (!localChanges.contains(path)) {
                    addActionsForChange(path, change);
                }
            }
            break;
        }
    }
    for (auto it = remoteSubFolders.cbegin(); it != remoteSubFolders.cend(); ++it) {
        foldersToScan.append({ it.key(), it.value() });
    }
    for (const auto& path : localSubFolders) {
        if (!remoteSubFolders.contains(path)) {
            foldersToScan.append({ path, RemoteUnchanged });
        }
    }
}

void DirectorySynchronizerPrivate::addActionsForChange(
        const QString& path, const DirectorySynchronizerPrivate::Change& change)
{
    switch (change.type) {
    case Change::Unchanged:
        // Not a change. Add it so we get warned when new types are added and we don't handle them.
        qCWarning(log) << "Invalid - we should never get a Change::Unchanged in " << __func__;
        break;
    case Change::AddedLocally:
        addSyncAction(new UploadSyncAction(path, SyncStateEntry(), change.lastModified));
        break;
    case Change::CreatedDirLocally:
        addSyncAction(new MkDirRemoteSyncAction(path));
        remoteFoldersToCreate << path;
        break;
    case Change::ChangedLocally:
        addSyncAction(
                new UploadSyncAction(path, syncStateDatabase->getEntry(path), change.lastModified));
        break;
    case Change::DeletedLocally:
        addSyncAction(new DeleteRemoteSyncAction(path, syncStateDatabase->getEntry(path)));
        break;
    case Change::AddedRemotely:
        addSyncAction(new DownloadSyncAction(path, SyncStateEntry(), change.syncProperty));
        break;
    case Change::CreatedDirRemotely:
        addSyncAction(new MkDirLocalSyncAction(path, change.syncProperty));
        break;
    case Change::ChangedRemotely:
        addSyncAction(new DownloadSyncAction(path, syncStateDatabase->getEntry(path),
                                             change.syncProperty));
        break;
    case Change::DeletedRemotely:
        addSyncAction(new DeleteLocalSyncAction(path, syncStateDatabase->getEntry(path)));
        break;
    }
}

void DirectorySynchronizerPrivate::addSyncAction(DirectorySynchronizerPrivate::SyncAction* action)
{
    syncActionsToRun << QSharedPointer<SyncAction>(action);
}

/**
 * @brief Run any local actions that do not require server interaction.
 */
void DirectorySynchronizerPrivate::runLocalActions()
{
    decltype(syncActionsToRun) remainingSyncActions;
    for (const auto& action : qAsConst(syncActionsToRun)) {
        switch (action->type) {
        case MkDirLocal: {
            QDir dir(localDirectoryPath + "/" + action->path);
            if (!dir.mkpath(".")) {
                setError(SynchronizerError::FailedCreatingLocalFolder,
                         tr("Creating folder %1 has failed").arg(dir.absolutePath()));
                return;
            }
            if (!syncStateDatabase->addEntry(SyncStateEntry(
                        action->path, QDateTime(),
                        qSharedPointerCast<MkDirLocalSyncAction>(action)->syncAttribute))) {
                setError(SynchronizerError::SyncStateDatabaseWriteFailed,
                         tr("Failed to write an entry to the sync state database"));
                return;
            }
            break;
        }
        case DeleteLocal:
            if (!deleteLocally(action->path)) {
                return;
            } else {
                if (!syncStateDatabase->removeEntries(action->path)
                    || !syncStateDatabase->removeEntry(action->path)) {
                    setError(SynchronizerError::SyncStateDatabaseDeleteFailed,
                             tr("Failed to delete entries from the sync state database"));
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
        setError(SynchronizerError::Stuck, tr("Cannot continue sync - it is stuck"));
        return;
    }

    syncActionsToRun = remainingSyncActions;

    if (syncActionsToRun.isEmpty() && runningJobs <= 0) {
        if (error == SynchronizerError::NoError) {
            // Safe remote folder sync attributes
            for (auto it = remoteFoldersSyncAttributes.cbegin();
                 it != remoteFoldersSyncAttributes.cend(); ++it) {
                if (!syncStateDatabase->addEntry(
                            SyncStateEntry(it.key(), QDateTime(), it.value()))) {
                    setError(SynchronizerError::SyncStateDatabaseWriteFailed,
                             tr("Failed to write folder sync attribute to sync state database"));
                    break;
                }
            }
        }
        finishLater();
    }
}

bool DirectorySynchronizerPrivate::deleteLocally(const QString& path)
{
    auto fullPath = localDirectoryPath + "/" + path;
    QFileInfo fi(fullPath);
    if (fi.isFile()) {
        QFile file(fullPath);
        if (!file.remove()) {
            setError(SynchronizerError::FailedDeletingLocalFile,
                     tr("Failed deleting local file %1: %2")
                             .arg(fi.absolutePath(), file.errorString()));
            return false;
        }
    } else {
        QDir dir(fullPath);
        {
            QDirIterator it(fullPath, QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                auto next = it.next();
                auto entryPath = SyncStateEntry::makePath("/" + dir.relativeFilePath(next));
                if (filter(entryPath, FileInfo::fromLocalFile(next))) {
                    QFile file(entryPath);
                    if (!file.remove()) {
                        setError(SynchronizerError::FailedDeletingLocalFile,
                                 tr("Failed deleting local file %1: %2")
                                         .arg(next, file.errorString()));
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
                auto entryPath = SyncStateEntry::makePath("/" + dir.relativeFilePath(next));
                if (filter(entryPath, FileInfo::fromLocalFile(next))) {
                    if (!QDir(next).rmdir(".")) {
                        setError(SynchronizerError::FailedDeletingLocalFolder,
                                 tr("Failed deleting local folder %1").arg(next));
                        return false;
                    }
                }
            }
        }
        if (!dir.rmdir(".")) {
            setError(SynchronizerError::FailedDeletingLocalFolder,
                     tr("Failed deleting local folder %1").arg(dir.absolutePath()));
            return false;
        }
    }
    return true;
}

/**
 * @brief Check if the action can be run.
 *
 * This is the case, if no pending remote mkdir job is running which creates a path which is a
 * prefix of the action's path.
 */
bool DirectorySynchronizerPrivate::canRunAction(
        const QSharedPointer<DirectorySynchronizerPrivate::SyncAction>& action)
{
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

void DirectorySynchronizerPrivate::runRemoteAction(
        const QSharedPointer<DirectorySynchronizerPrivate::SyncAction>& action)
{
    switch (action->type) {
    case MkDirLocal:
    case DeleteLocal:
        // There shouldn't be any such actions left
        qCWarning(log) << "We should not have any MkDirLocal and DeleteLocal actions in"
                       << __func__;
        break;
    case Upload: {
        qCDebug(log) << "Uploading" << action->path;
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
                if (!syncStateDatabase->addEntry(SyncStateEntry(uploadAction->path,
                                                                uploadAction->lastModified,
                                                                job->fileInfo().syncAttribute()))) {
                    setError(SynchronizerError::SyncStateDatabaseWriteFailed,
                             tr("Failed to write to the sync state database"));
                    return;
                }
                break;
            case JobError::SyncAttributeMismatch:
                // There was a lost update (i.e. another client uploaded meanwhile).
                break;
            default:
                setError(SynchronizerError::UploadFailed,
                         tr("Uploading %1 failed: %2").arg(uploadAction->path, job->errorString()));
                return;
            }
            runRemoteActions();
        });
        job->start();
        break;
    }

    case Download: {
        qCDebug(log) << "Downloading" << action->path;
        ++runningJobs;
        auto job = jobFactory->downloadFile(this);
        job->setRemoteFilename(remoteDirectoryPath + "/" + action->path);
        auto saveFile = new QSaveFile(job);
        saveFile->setFileName(localDirectoryPath + "/" + action->path);
        if (!saveFile->open(QIODevice::WriteOnly)) {
            setError(SynchronizerError::OpeningLocalFileFailed,
                     tr("Opening file %1 for reading/writing failed: %2")
                             .arg(saveFile->fileName(), saveFile->errorString()));
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
                                 tr("Failed to write to sync state database"));
                        return;
                    }
                } else {
                    setError(SynchronizerError::WritingToLocalFileFailed,
                             tr("Failed to commit downloaded data to file %1: %2")
                                     .arg(saveFile->fileName(), saveFile->errorString()));
                    return;
                }
                break;
            default:
                setError(SynchronizerError::DownloadFailed,
                         tr("Downloading %1 failed: %2")
                                 .arg(downloadAction->path, job->errorString()));
                return;
            }
            runRemoteActions();
        });
        job->start();
        break;
    }

    case DeleteRemote: {
        qCDebug(log) << "Deleting remote" << action->path;
        ++runningJobs;
        auto job = jobFactory->deleteResource(this);
        job->setPath(remoteDirectoryPath + "/" + action->path);
        setupDefaultJobSignals(job);
        connect(job, &AbstractJob::finished, this, [=]() {
            --runningJobs;
            switch (job->error()) {
            case JobError::NoError:
                syncStateDatabase->removeEntries(action->path);
                syncStateDatabase->removeEntry(action->path);
                break;
            default:
                setError(SynchronizerError::FailedDeletingRemoteResource,
                         tr("Failed deleting remote resource %1").arg(action->path));
                return;
            }
            runRemoteActions();
        });
        job->start();
        break;
    }

    case MkDirRemote: {
        qCDebug(log) << "Creating remote folder" << action->path;
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
                                 .arg(action->path, job->errorString()));
                return;
            }
            runRemoteActions();
        });
        job->start();
        break;
    }
    }
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
                             tr("Failed to close the sync state database"));
                }
            }
            state = SynchronizerState::Finished;
            emit q->finished();
        }
    });
}

void DirectorySynchronizerPrivate::setError(SynchronizerError error, const QString& errorString)
{
    if (this->error == SynchronizerError::NoError) {
        this->error = error;
        this->errorString = errorString;
    }
    finishLater();
}

void DirectorySynchronizerPrivate::createRemoteFolder()
{
    qCDebug(log) << "Creating remote folder";
    remoteFolderPartsToCreate = remoteDirectoryPath.split("/", Qt::SkipEmptyParts);
    createdRemoteFolderParts.clear();
    createNextRemoteFolderPart();
}

void DirectorySynchronizerPrivate::createSyncPlan()
{
    qCDebug(log) << "Creating sync plan";
    runningJobs = 0;
    // First, we try to get the sync propterty of the remote folder. By this, we first ensure it
    // is there and we will learn if there were any changes remotely.
    auto job = jobFactory->getFileInfo(this);
    job->setPath(remoteDirectoryPath);
    setupDefaultJobSignals(job);
    connect(job, &GetFileInfoJob::finished, this, [=]() {
        switch (job->error()) {
        case JobError::NoError: {
            // Remote folder exists! Save the sync attribute (we store it in the DB later after
            // a successful sync) and start traversing the folder structure:
            auto fileInfo = job->fileInfo();
            remoteFoldersSyncAttributes["/"] = fileInfo.syncAttribute();
            auto entry = syncStateDatabase->getEntry("/");
            foldersToScan.append({ "/",
                                   entry.syncProperty() == fileInfo.syncAttribute()
                                           ? RemoteUnchanged
                                           : RemoteChanged });
            scanNextFolder();
            break;
        }
        default:
            setError(SynchronizerError::FailedGettingRemoteFolder,
                     tr("Failed to get information about the remote folder: %1")
                             .arg(job->errorString()));
            break;
        }
    });
    job->start();
}

void DirectorySynchronizerPrivate::executeSyncPlan()
{
    qCDebug(log) << "Executing sync plan";
    if (error != SynchronizerError::NoError) {
        finishLater();
        return;
    }

    qCDebug(log) << "Running local sync actions";
    runLocalActions();

    if (error == SynchronizerError::NoError) {
        qCDebug(log) << "Running remote sync actions";
        runRemoteActions();
    } else {
        finishLater();
    }
}

DirectorySynchronizerPrivate::UploadSyncAction::UploadSyncAction(const QString& path,
                                                                 const SyncStateEntry& entry,
                                                                 const QDateTime& lastModified)
    : SyncAction(Upload, path), previousSyncEntry(entry), lastModified(lastModified)
{
}

DirectorySynchronizerPrivate::SyncAction::SyncAction(
        DirectorySynchronizerPrivate::SyncActionType type, const QString& path)
    : type(type), path(path)
{
}

DirectorySynchronizerPrivate::DeleteRemoteSyncAction::DeleteRemoteSyncAction(
        const QString& path, const SyncStateEntry& entry)
    : SyncAction(DeleteRemote, path), previousSyncEntry(entry)
{
}

DirectorySynchronizerPrivate::MkDirLocalSyncAction::MkDirLocalSyncAction(
        const QString& path, const QString& syncAttribute)
    : SyncAction(MkDirLocal, path), syncAttribute(syncAttribute)
{
}

DirectorySynchronizerPrivate::MkDirRemoteSyncAction::MkDirRemoteSyncAction(const QString& path)
    : SyncAction(MkDirRemote, path)
{
}

DirectorySynchronizerPrivate::DownloadSyncAction::DownloadSyncAction(const QString& path,
                                                                     const SyncStateEntry& entry,
                                                                     const QString& syncAttribute)
    : SyncAction(Download, path), previousSyncEntry(entry), syncAttribute(syncAttribute)
{
}

DirectorySynchronizerPrivate::DeleteLocalSyncAction::DeleteLocalSyncAction(
        const QString& path, const SyncStateEntry& entry)
    : SyncAction(DeleteLocal, path), previousSyncEntry(entry)
{
}

} // namespace SynqClient
