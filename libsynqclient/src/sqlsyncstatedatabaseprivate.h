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

#ifndef SYNQCLIENT_SQLSYNCSTATEDATABASEPRIVATE_H
#define SYNQCLIENT_SQLSYNCSTATEDATABASEPRIVATE_H

#include <tuple>

#include <QSqlDatabase>

#include "syncstatedatabaseprivate.h"
#include "SynqClient/sqlsyncstatedatabase.h"

namespace SynqClient {

class SQLSyncStateDatabasePrivate : public SyncStateDatabasePrivate
{
public:
    enum class SplitPathMode { NameIncluded, NameExcluded };

    explicit SQLSyncStateDatabasePrivate(SQLSyncStateDatabase* q);

    Q_DECLARE_PUBLIC(SQLSyncStateDatabase);

    QString dbConnName;
    QString defaultDbConnName;

    /**
     * @brief Auto-removal of connections to SQLite DBs.
     *
     * This flag is used to automatically remove connections to SQLite DBs if they were added by a
     * path to the DB file.
     */
    bool removeDb;

    bool initializeDbV1();
    void removeOldConnection();
    QSqlDatabase getDb() const;

    std::tuple<QString, QString> splitPath(const QString& path,
                                           SplitPathMode mode = SplitPathMode::NameIncluded);
};

} // namespace SynqClient

#endif // SYNQCLIENT_SQLSYNCSTATEDATABASEPRIVATE_H
