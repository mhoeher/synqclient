/*
 * Copyright 2021 Martin Hoeher <martin@rpdev.net>
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
#ifndef SYNQCLIENT_SYNCACTIONS_H
#define SYNQCLIENT_SYNCACTIONS_H

#include <QDateTime>

#include "SynqClient/syncstateentry.h"

namespace SynqClient {

enum SyncActionType { Upload, Download, DeleteLocal, DeleteRemote, MkDirLocal, MkDirRemote };

struct SyncAction
{
    SyncActionType type;
    QString path;
    int retries;

    SyncAction(SyncActionType type, const QString& path)
        : type(type), path(SyncStateEntry::makePath(path)), retries(0)
    {
    }
};

struct UploadSyncAction : public SyncAction
{
    SyncStateEntry previousSyncEntry;
    QDateTime lastModified;

    UploadSyncAction(const QString& path, const SyncStateEntry& entry,
                     const QDateTime& lastModified)
        : SyncAction(Upload, path), previousSyncEntry(entry), lastModified(lastModified)
    {
    }
};

struct DownloadSyncAction : public SyncAction
{
    SyncStateEntry previousSyncEntry;
    QString syncAttribute;

    DownloadSyncAction(const QString& path, const SyncStateEntry& entry,
                       const QString& syncAttribute)
        : SyncAction(Download, path), previousSyncEntry(entry), syncAttribute(syncAttribute)
    {
    }
};

struct DeleteRemoteSyncAction : public SyncAction
{
    SyncStateEntry previousSyncEntry;

    DeleteRemoteSyncAction(const QString& path, const SyncStateEntry& entry)
        : SyncAction(DeleteRemote, path), previousSyncEntry(entry)
    {
    }
};

struct DeleteLocalSyncAction : public SyncAction
{
    SyncStateEntry previousSyncEntry;

    DeleteLocalSyncAction(const QString& path, const SyncStateEntry& entry)
        : SyncAction(DeleteLocal, path), previousSyncEntry(entry)
    {
    }
};

struct MkDirLocalSyncAction : SyncAction
{
    QString syncAttribute;

    explicit MkDirLocalSyncAction(const QString& path, const QString& syncAttribute)
        : SyncAction(MkDirLocal, path), syncAttribute(syncAttribute)
    {
    }
};

struct MkDirRemoteSyncAction : SyncAction
{
    explicit MkDirRemoteSyncAction(const QString& path) : SyncAction(MkDirRemote, path) {}
};

}

#endif // SYNQCLIENT_SYNCACTIONS_H
