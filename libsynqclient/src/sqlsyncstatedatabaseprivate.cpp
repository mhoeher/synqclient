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

#include "sqlsyncstatedatabaseprivate.h"

#include <QLoggingCategory>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QUuid>

namespace SynqClient {

static Q_LOGGING_CATEGORY(log, "SynqClient.SQLSyncStateDatabase", QtWarningMsg);

SQLSyncStateDatabasePrivate::SQLSyncStateDatabasePrivate(SQLSyncStateDatabase* q)
    : SyncStateDatabasePrivate(q),
      dbConnName(),
      defaultDbConnName(QUuid::createUuid().toString()),
      removeDb(false)
{
}

bool SQLSyncStateDatabasePrivate::initializeDbV1()
{
    QSqlQuery query(getDb());
    if (!query.prepare("CREATE TABLE IF NOT EXISTS "
                       "version (key string PRIMARY KEY, value);")) {
        qCWarning(log) << "Failed to prepare query:" << query.lastError().text();
        return false;
    }
    if (!query.exec()) {
        qCWarning(log) << "Failed to create version table:" << query.lastError().text();
        return false;
    }
    if (!query.prepare("SELECT value FROM version WHERE key == 'version';")) {
        qCWarning(log) << "Failed to prepare query:" << query.lastError().text();
        return false;
    }
    int version = 0;
    if (query.exec()) {
        if (query.first()) {
            auto record = query.record();
            version = record.value("value").toInt();
        }
    } else {
        qCWarning(log) << "Failed to get version of sync DB:" << query.lastError().text();
        return false;
    }
    if (version == 0) {
        // Note: We call the syncProperty "etag" - this is to be compatible
        // with OpenTodoList, where this code has been factored out from.
        if (!query.prepare("CREATE TABLE files ("
                           "`parent` string, "
                           "`entry` string NOT NULL, "
                           "`modificationDate` date not null, "
                           "`etag` string not null, "
                           "PRIMARY KEY(`parent`, `entry`)"
                           ");")) {
            qCWarning(log) << "Failed to prepare query:" << query.lastError().text();
            return false;
        }
        if (!query.exec()) {
            qCWarning(log) << "Failed to create files table:" << query.lastError().text();
            return false;
        }
        if (!query.prepare("INSERT OR REPLACE INTO version(key, value) "
                           "VALUES ('version', 1);")) {
            qCWarning(log) << "Failed to prepare query:" << query.lastError().text();
            return false;
        }
        if (!query.exec()) {
            qCWarning(log) << "Failed to insert version into DB:" << query.lastError().text();
            return false;
        }
    }
    return true;
}

void SQLSyncStateDatabasePrivate::removeOldConnection()
{
    if (removeDb) {
        QSqlDatabase::removeDatabase(dbConnName);
    }
    removeDb = false;
    dbConnName = QString();
}

QSqlDatabase SQLSyncStateDatabasePrivate::getDb() const
{
    QSqlDatabase db;
    if (!dbConnName.isNull()) {
        db = QSqlDatabase::database(dbConnName);
    }
    return db;
}

std::tuple<QString, QString> SQLSyncStateDatabasePrivate::splitPath(const QString& path,
                                                                    SplitPathMode mode)
{
    QString parent = "";
    QString name = "";
    auto cleanPath = SyncStateEntry::makePath(path);

    if (cleanPath == "/") {
        return std::make_tuple(parent, name);
    }

    auto parts = cleanPath.split("/", Qt::SkipEmptyParts);

    if (mode == SplitPathMode::NameIncluded) {
        if (parts.length() > 0) {
            name = parts.last();
            parts.pop_back();
        }
    }

    parent = parts.join("/");
    if (parent.isNull()) {
        parent = "";
    }
    return std::make_tuple(parent, name);
}

} // namespace SynqClient
