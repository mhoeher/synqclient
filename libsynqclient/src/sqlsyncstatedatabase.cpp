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

#include "../inc/sqlsyncstatedatabase.h"

#include <QDateTime>
#include <QLoggingCategory>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include "sqlsyncstatedatabaseprivate.h"

namespace SynqClient {

static Q_LOGGING_CATEGORY(log, "SynqClient.SQLSyncStateDatabase", QtWarningMsg);

/**
 * @class SQLSyncStateDatabase
 * @brief Store sync state information in an SQL database.
 *
 * This class can be used to store the sync state information in an SQL database. This can either
 * be a dedicated database (such as a single SQLite database file) or a database in an SQL
 * server such as MySQL, PostgreSQL and so on.
 */

/**
 * @brief Constructor.
 */
SQLSyncStateDatabase::SQLSyncStateDatabase(QObject* parent)
    : SyncStateDatabase(new SQLSyncStateDatabasePrivate(this), parent)
{
}

/**
 * @brief Constructor.
 *
 * This is an overloaded version of the constructor. It accepts the database to
 * be used to store state.
 *
 * @sa setDatabase()
 */
SQLSyncStateDatabase::SQLSyncStateDatabase(const QSqlDatabase& db, QObject* parent)
    : SyncStateDatabase(new SQLSyncStateDatabasePrivate(this), parent)
{
    setDatabase(db);
}

/**
 * @brief Constructor.
 *
 * This is an overloaded version of the constructor. It accepts the path to the SQLite database
 * file to be usd to store data.
 */
SQLSyncStateDatabase::SQLSyncStateDatabase(const QString& path, QObject* parent)
    : SyncStateDatabase(new SQLSyncStateDatabasePrivate(this), parent)
{
    setDatabase(path);
}

/**
 * @brief Destructor.
 */
SQLSyncStateDatabase::~SQLSyncStateDatabase() {}

/**
 * @brief The SQL database used to store state.
 */
QSqlDatabase SQLSyncStateDatabase::database() const
{
    Q_D(const SQLSyncStateDatabase);
    return d->db;
}

/**
 * @brief Set the SQL database used to store state.
 */
void SQLSyncStateDatabase::setDatabase(const QSqlDatabase& database)
{
    Q_D(SQLSyncStateDatabase);
    d->db = database;
}

/**
 * @brief Set the path to the SQLite datbase file to be used to store state.
 *
 * This is an overloaded setter. When being used, it will create a new SQLite database
 * connection which uses the file @p path to store information.
 */
void SQLSyncStateDatabase::setDatabase(const QString& path)
{
    auto db = QSqlDatabase::addDatabase("QSQLITE", path);
    db.setDatabaseName(path);
    setDatabase(db);
}

/**
 * @brief Constructor.
 */
SQLSyncStateDatabase::SQLSyncStateDatabase(SQLSyncStateDatabasePrivate* d, QObject* parent)
    : SyncStateDatabase(d, parent)
{
}

/**
 * @brief Open the database.
 *
 * This opens the sync state database. If the underlying SQL database connection is not yet open,
 * this will also open that connection.
 *
 * Returns true on success or false on error.
 */
bool SQLSyncStateDatabase::openDatabase()
{
    Q_D(SQLSyncStateDatabase);
    if (isOpen()) {
        qCWarning(log) << "Database is already open";
        return false;
    }
    if (!d->db.isValid()) {
        qCWarning(log) << "No valid database connection is set";
        return false;
    }
    if (!d->db.isOpen()) {
        qCDebug(log) << "Database connection not yet open - going to open it";
        if (!d->db.open()) {
            qCWarning(log) << "Failed to open database:" << d->db.lastError().text();
            return false;
        }
    }
    if (!d->initializeDbV1()) {
        return false;
    }
    setOpen(true);
    return true;
}

bool SQLSyncStateDatabase::addEntry(const SyncStateEntry& entry)
{
    Q_D(SQLSyncStateDatabase);
    if (!entry.isValid()) {
        return false;
    }
    auto parts = d->splitPath(entry.path());

    QSqlQuery query(d->db);
    query.prepare("INSERT OR REPLACE INTO files "
                  "(parent, entry, modificationDate, etag) "
                  "VALUES (?, ?, ?, ?);");
    query.addBindValue(std::get<0>(parts));
    query.addBindValue(std::get<1>(parts));
    query.addBindValue(entry.modificationTime());
    query.addBindValue(entry.syncProperty());
    if (!query.exec()) {
        qCWarning(log) << "Failed to insert SyncDB entry:" << query.lastError().text();
        return false;
    }
    return true;
}

SyncStateEntry SQLSyncStateDatabase::getEntry(const QString& path)
{
    Q_D(SQLSyncStateDatabase);

    SyncStateEntry result;
    QSqlQuery query(d->db);
    query.prepare("SELECT parent, entry, modificationDate, etag "
                  "FROM files WHERE parent = ? and entry = ?;");
    auto dbPath = d->splitPath(path);
    query.addBindValue(std::get<0>(dbPath));
    query.addBindValue(std::get<1>(dbPath));
    if (query.exec()) {
        while (query.next()) {
            SyncStateEntry entry;
            auto record = query.record();
            result.setPath("/" + record.value("parent").toString() + "/"
                           + record.value("entry").toString());
            result.setModificationTime(record.value("modificationDate").toDateTime());
            result.setSyncProperty(record.value("etag").toString());
            result.setValid(true);
            break;
        }
    } else {
        qCWarning(log) << "Failed to get entry from DB:" << query.lastError().text();
    }
    return result;
}

QVector<SyncStateEntry> SQLSyncStateDatabase::findEntries(const QString& parent, bool* ok)
{
    Q_D(SQLSyncStateDatabase);
    bool status = false;

    QVector<SyncStateEntry> result;
    QSqlQuery query(d->db);
    query.prepare("SELECT parent, entry, modificationDate, etag "
                  "FROM files WHERE parent = ?;");
    query.addBindValue(std::get<0>(
            d->splitPath(parent, SQLSyncStateDatabasePrivate::SplitPathMode::NameExcluded)));
    if (query.exec()) {
        while (query.next()) {
            SyncStateEntry entry;
            auto record = query.record();
            entry.setPath("/" + record.value("parent").toString() + "/"
                          + record.value("entry").toString());
            entry.setModificationTime(record.value("modificationDate").toDateTime());
            entry.setSyncProperty(record.value("etag").toString());
            entry.setValid(true);
            result << entry;
        }
        status = true;
    } else {
        qCWarning(log) << "Failed to get sync entries from DB:" << query.lastError().text();
    }
    if (ok) {
        *ok = status;
    }
    return result;
}

bool SQLSyncStateDatabase::removeEntries(const QString& path)
{
    Q_D(SQLSyncStateDatabase);
    QSqlQuery query(d->db);
    query.prepare("DELETE FROM files "
                  "WHERE parent LIKE ? || '%' OR (parent = ? AND entry = ?);");
    auto dbPath = d->splitPath(path);
    auto parent = std::get<0>(dbPath);
    auto entry = std::get<1>(dbPath);
    query.addBindValue(std::get<0>(
            d->splitPath(path, SQLSyncStateDatabasePrivate::SplitPathMode::NameExcluded)));
    query.addBindValue(parent);
    query.addBindValue(entry);
    if (!query.exec()) {
        qCWarning(log) << "Failed to delete directory from "
                          "sync DB:"
                       << query.lastError().text();
        return false;
    }
    return true;
}

bool SQLSyncStateDatabase::removeEntry(const QString& path)
{
    Q_D(SQLSyncStateDatabase);
    QSqlQuery query(d->db);
    query.prepare("DELETE FROM files "
                  "WHERE parent = ? AND entry = ?;");
    auto dbPath = d->splitPath(path);
    auto parent = std::get<0>(dbPath);
    auto entry = std::get<1>(dbPath);
    query.addBindValue(parent);
    query.addBindValue(entry);
    if (!query.exec()) {
        qCWarning(log) << "Failed to delete entry from "
                          "sync DB:"
                       << query.lastError().text();
        return false;
    }
    return true;
}

/**
 * @brief Close the database.
 *
 * This closes the sync state database. Internally, this will close the connection to the underlying
 * SQL database.
 *
 * Returns true on success or false otherwise.
 */
bool SQLSyncStateDatabase::closeDatabase()
{
    Q_D(SQLSyncStateDatabase);
    if (!isOpen()) {
        qCWarning(log) << "Database is not open";
        return false;
    }
    if (d->db.isOpen()) {
        d->db.close();
    }
    setOpen(false);
    return true;
}

} // namespace SynqClient
