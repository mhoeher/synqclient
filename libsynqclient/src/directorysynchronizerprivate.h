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

#ifndef SYNQCLIENT_DIRECTORYSYNCHRONIZERPRIVATE_H
#define SYNQCLIENT_DIRECTORYSYNCHRONIZERPRIVATE_H

#include <QCoreApplication>
#include <QDateTime>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QSet>
#include <QSharedPointer>
#include <QQueue>

#include "SynqClient/abstractjob.h"
#include "changetree.h"
#include "SynqClient/directorysynchronizer.h"
#include "SynqClient/libsynqclient.h"
#include "SynqClient/syncstateentry.h"
#include "syncactions.h"

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
    bool retryWithFewerJobs;
    SyncConflictStrategy syncConflictStrategy;
    SynchronizerFlags flags;
    bool stopped;
    int progress;
    int numTotalSyncActionsToRun;

    void finishLater();
    void setError(SynchronizerError error, const QString& errorString, JobError jobError);

    // Sync Stages:
    void createRemoteFolder();
    void createSyncPlan();
    void executeSyncPlan();

    // General resources
    QMap<QString, QString> remoteFoldersSyncAttributes;
    int runningJobs;
    void setupDefaultJobSignals(AbstractJob* job);
    static QMap<QString, SyncStateEntry> syncStateListToMap(const QVector<SyncStateEntry>& list);

    // Create remote folder stage
    QStringList createdRemoteFolderParts;
    QStringList remoteFolderPartsToCreate;
    void createNextRemoteFolderPart();

    // Create sync plan stage
    ChangeTree buildLocalChangeTree();
    void buildRemoteChangeTree();
    void mergeChangeTrees();
    void mergeChangeNodes(const QString& path, const ChangeTreeNode* localChange,
                          const ChangeTreeNode* remoteChange);
    void mergeChangeNodesLocalWins(const QString& path, const ChangeTreeNode& localChange,
                                   const ChangeTreeNode& remoteChange);
    void mergeChangeNodesRemoteWins(const QString& path, const ChangeTreeNode& localChange,
                                    const ChangeTreeNode& remoteChange);

    ChangeTree localChangeTree;
    ChangeTree remoteChangeTree;
    QQueue<QString> remoteFoldersToScan;

    // Execute sync stage
    QVector<QSharedPointer<SyncAction>> syncActionsToRun;
    QStringList remoteFoldersToCreate;
    QStringList remoteResourcesToDelete;

    void addSyncAction(SyncAction* action);
    void runLocalActions();
    void runRemoteActions();
    bool deleteLocally(const QString& path);
    bool canRunAction(const QSharedPointer<SyncAction>& action);
    void runRemoteAction(const QSharedPointer<SyncAction>& action);

    void updateProgress();

signals:

    void stopRequested();
};

} // namespace SynqClient

#endif // SYNQCLIENT_DIRECTORYSYNCHRONIZERPRIVATE_H
