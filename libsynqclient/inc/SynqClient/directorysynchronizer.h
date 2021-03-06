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

#ifndef SYNQCLIENT_DIRECTORYSYNCHRONIZER_H
#define SYNQCLIENT_DIRECTORYSYNCHRONIZER_H

#include <functional>

#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>

#include "FileInfo"
#include "libsynqclient.h"
#include "libsynqclient_global.h"

namespace SynqClient {

class AbstractJobFactory;
class SyncStateDatabase;

class DirectorySynchronizerPrivate;

class LIBSYNQCLIENT_EXPORT DirectorySynchronizer : public QObject
{
    Q_OBJECT
public:
    typedef std::function<bool(const QString& path, const FileInfo& fileInfo)> Filter;

    explicit DirectorySynchronizer(QObject* parent = nullptr);
    ~DirectorySynchronizer() override;

    AbstractJobFactory* jobFactory() const;
    void setJobFactory(AbstractJobFactory* jobFactory);

    SyncStateDatabase* syncStateDatabase() const;
    void setSyncStateDatabase(SyncStateDatabase* syncStateDatabase);

    QString localDirectoryPath() const;
    void setLocalDirectoryPath(const QString& localDirectoryPath);

    QString remoteDirectoryPath() const;
    void setRemoteDirectoryPath(const QString& remoteDirectoryPath);

    Filter filter() const;
    void setFilter(const Filter& filter);

    int maxJobs() const;
    void setMaxJobs(int maxJobs);

    bool retryWithFewerJobs() const;

    SyncConflictStrategy syncConflictStrategy() const;
    void setSyncConflictStrategy(SyncConflictStrategy strategy);

    SynchronizerFlags flags() const;
    void setFlags(const SynchronizerFlags flags);

    SynchronizerState state() const;
    SynchronizerError error() const;
    QString errorString() const;

    void start();
    void stop();

signals:

    void finished();
    void logMessageAvailable(SynchronizerLogEntryType type, const QString& message);
    void progress(int value);

protected:
    explicit DirectorySynchronizer(DirectorySynchronizerPrivate* d, QObject* parent = nullptr);

    QScopedPointer<DirectorySynchronizerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DirectorySynchronizer);
};

} // namespace SynqClient

#endif // SYNQCLIENT_DIRECTORYSYNCHRONIZER_H
