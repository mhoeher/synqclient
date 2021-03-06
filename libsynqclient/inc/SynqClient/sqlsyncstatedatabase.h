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

#ifndef SYNQCLIENT_SQLSYNCSTATEDATABASE_H
#define SYNQCLIENT_SQLSYNCSTATEDATABASE_H

#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>

#include "SyncStateDatabase"
#include "libsynqclient_global.h"

class QSqlDatabase;

namespace SynqClient {

class SQLSyncStateDatabasePrivate;

class LIBSYNQCLIENT_EXPORT SQLSyncStateDatabase : public SyncStateDatabase
{
    Q_OBJECT
public:
    explicit SQLSyncStateDatabase(QObject* parent = nullptr);
    explicit SQLSyncStateDatabase(const QSqlDatabase& db, QObject* parent = nullptr);
    explicit SQLSyncStateDatabase(const QString& path, QObject* parent = nullptr);
    ~SQLSyncStateDatabase() override;

    QSqlDatabase database() const;
    void setDatabase(const QSqlDatabase& database);
    void setDatabase(const QString& path);

protected:
    explicit SQLSyncStateDatabase(SQLSyncStateDatabasePrivate* d, QObject* parent = nullptr);

    Q_DECLARE_PRIVATE(SQLSyncStateDatabase);

    // SyncStateDatabase interface
public:
    bool openDatabase() override;
    bool addEntry(const SyncStateEntry& entry) override;
    SyncStateEntry getEntry(const QString& path) override;
    QVector<SyncStateEntry> findEntries(const QString& parent, bool* ok) override;
    bool removeEntries(const QString& path) override;
    bool removeEntry(const QString& path) override;
    bool closeDatabase() override;
};

} // namespace SynqClient

#endif // SYNQCLIENT_SQLSYNCSTATEDATABASE_H
