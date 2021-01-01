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

#ifndef SYNQCLIENT_DIRECTORYSYNCHRONIZERPRIVATE_H
#define SYNQCLIENT_DIRECTORYSYNCHRONIZERPRIVATE_H

#include <QCoreApplication>
#include <QDateTime>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QSet>
#include <QSharedPointer>

#include "abstractjob.h"
#include "directorysynchronizer.h"
#include "libsynqclient.h"
#include "syncstateentry.h"

namespace SynqClient {

class DirectorySynchronizerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DirectorySynchronizerPrivate(DirectorySynchronizer* q);

    DirectorySynchronizer* q_ptr;
    Q_DECLARE_PUBLIC(DirectorySynchronizer);

    QPointer<AbstractJobFactory> jobFactory;
    QPointer<SyncStateDatabase> syncStateDatabase;
    QString localDirectoryPath;
    QString remoteDirectoryPath;
    DirectorySynchronizer::Filter filter;
    SynchronizerState state;
    SynchronizerError error;
    QString errorString;
    int maxJobs;
    SyncConflictStrategy syncConflictStrategy;
    SynchronizerFlags flags;
    bool stopped;

    // General resources
    QMap<QString, QString> remoteFoldersSyncAttributes;
    int runningJobs;
    void setupDefaultJobSignals(AbstractJob* job);
    static QMap<QString, SyncStateEntry> syncStateListToMap(const QVector<SyncStateEntry>& list);

    void finishLater();
    void setError(SynchronizerError error, const QString& errorString);

    void createRemoteFolder();
    void createSyncPlan();
    void executeSyncPlan();

    // Create remote folder state
    QStringList createdRemoteFolderParts;
    QStringList remoteFolderPartsToCreate;
    void createNextRemoteFolderPart();

    // Create sync plan state
    enum RemoteEntryState { RemoteUnchanged, RemoteChanged };

    struct ScanRecord
    {
        QString path;
        RemoteEntryState remoteState;
    };

    struct Change
    {
        enum Type {
            Unchanged,
            AddedLocally,
            CreatedDirLocally,
            ChangedLocally,
            DeletedLocally,
            AddedRemotely,
            CreatedDirRemotely,
            ChangedRemotely,
            DeletedRemotely
        };

        Type type = Unchanged;
        QDateTime lastModified = QDateTime();
        QString syncProperty = QString();
    };

    QVector<ScanRecord> foldersToScan;
    void scanNextFolder();
    void scanLocalFolderOnly(const ScanRecord& folder);
    void scanLocalAndRemoteFolder(const ScanRecord& folder);
    QMap<QString, Change> findLocalChanges(const QMap<QString, SyncStateEntry> lastRunEntries,
                                           const QString& path, bool& ok,
                                           QSet<QString>& subFolders);
    QMap<QString, Change> findRemoteChanges(const QMap<QString, SyncStateEntry> lastRunEntries,
                                            const QVector<FileInfo>& remoteEntries,
                                            const QString& path, bool& ok,
                                            QMap<QString, RemoteEntryState>& subFolders);
    void resolveChanges(const QMap<QString, Change> localChanges,
                        const QMap<QString, Change> remoteChanges,
                        const QSet<QString>& localSubFolders,
                        const QMap<QString, RemoteEntryState>& remoteSubFolders);
    void addActionsForChange(const QString& path, const Change& change);

    // Execute sync stage
    enum SyncActionType { Upload, Download, DeleteLocal, DeleteRemote, MkDirLocal, MkDirRemote };

    struct SyncAction
    {
        SyncActionType type;
        QString path;

        SyncAction(SyncActionType type, const QString& path);
    };

    struct UploadSyncAction : public SyncAction
    {
        SyncStateEntry previousSyncEntry;
        QDateTime lastModified;

        UploadSyncAction(const QString& path, const SyncStateEntry& entry,
                         const QDateTime& lastModified);
    };

    struct DownloadSyncAction : public SyncAction
    {
        SyncStateEntry previousSyncEntry;
        QString syncAttribute;

        DownloadSyncAction(const QString& path, const SyncStateEntry& entry,
                           const QString& syncAttribute);
    };

    struct DeleteRemoteSyncAction : public SyncAction
    {
        SyncStateEntry previousSyncEntry;

        DeleteRemoteSyncAction(const QString& path, const SyncStateEntry& entry);
    };

    struct DeleteLocalSyncAction : public SyncAction
    {
        SyncStateEntry previousSyncEntry;

        DeleteLocalSyncAction(const QString& path, const SyncStateEntry& entry);
    };

    struct MkDirLocalSyncAction : SyncAction
    {
        QString syncAttribute;

        explicit MkDirLocalSyncAction(const QString& path, const QString& syncAttribute);
    };

    struct MkDirRemoteSyncAction : SyncAction
    {
        explicit MkDirRemoteSyncAction(const QString& path);
    };

    QVector<QSharedPointer<SyncAction>> syncActionsToRun;
    QStringList remoteFoldersToCreate;

    void addSyncAction(SyncAction* action);
    void runLocalActions();
    void runRemoteActions();
    bool deleteLocally(const QString& path);
    bool canRunAction(const QSharedPointer<SyncAction>& action);
    void runRemoteAction(const QSharedPointer<SyncAction>& action);

signals:

    void stopRequested();
};

} // namespace SynqClient

#endif // SYNQCLIENT_DIRECTORYSYNCHRONIZERPRIVATE_H
