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

#include "SynqClient/syncstatedatabase.h"

#include <QQueue>

#include "syncstatedatabaseprivate.h"

namespace SynqClient {

/**
 * @class SyncStateDatabase
 * @brief Store synchronization state information between sync runs.
 *
 * This class is an abstract base for classes which serve as databases for persistently
 * storing synchronization state information. In order to identify changes and decide if
 * files need to be updated, some state information must be stored between sync runs. This
 * class serves as interface for such classes, so applications can opt to use any type of
 * storage for such information.
 */
/**
 * @brief Constructor.
 */
SyncStateDatabase::SyncStateDatabase(QObject* parent)
    : QObject(parent), d_ptr(new SyncStateDatabasePrivate(this))
{
}

/**
 * @brief Destructor.
 */
SyncStateDatabase::~SyncStateDatabase() {}

/**
 * @brief Open the database for a sync run.
 *
 * This method is called once to indicate that a sync starts. On success, it shall
 * return true. When an error occurs, it shall return false.
 *
 * The default implementation does nothing. Concrete sub-classes may override this
 * method to acquire resources or initialize the database.
 *
 * @sa closeDatabase()
 */
bool SyncStateDatabase::openDatabase()
{
    setOpen(true);
    return true;
}

/**
 * @brief Write the @p entry to the database.
 *
 * This adds the given entry to the database (or overrides it, if it already is present).
 * Returns true on success or false otherwise.
 */
bool SyncStateDatabase::addEntry(const SyncStateEntry& entry)
{
    Q_UNUSED(entry);
    return false;
}

/**
 * @brief Get a single entry from the database.
 *
 * This method returns the entry having the given @p path. If no such entry is
 * stored in the database, an invalid entry is returned.
 */
SyncStateEntry SyncStateDatabase::getEntry(const QString& path)
{
    Q_UNUSED(path);
    return SyncStateEntry();
}

/**
 * @brief Get all direct children of the entry with the given @p path.
 *
 * This returns a list of state entries which are direct children of the entry identified
 * by the path. This only includes direct children.
 *
 * If @p ok is set to a valid pointer, this writes true to the pointed to variable to indicate
 * success or false otherwise.
 */
QVector<SyncStateEntry> SyncStateDatabase::findEntries(const QString& path, bool* ok)
{
    Q_UNUSED(path);
    if (ok) {
        *ok = false;
    }
    return QVector<SyncStateEntry>();
}

/**
 * @brief Recursively delete entries from the database.
 *
 * This recursively removes all items which are children of the item identified by the
 * @p path.
 *
 * Returns true on success or false of there was any error.
 */
bool SyncStateDatabase::removeEntries(const QString& path)
{
    Q_UNUSED(path);
    return false;
}

/**
 * @brief Remove a single entry from the database.
 *
 * This removes the entry identified by the @p path from the database. On success, this returns
 * true. On error, false is returned.
 *
 * @note If the path refers to a directory, this will only remove the information about
 *       the directory itself. Data about children will still be kept in the database.
 */
bool SyncStateDatabase::removeEntry(const QString& path)
{
    Q_UNUSED(path);
    return false;
}

/**
 * @brief Close the database.
 *
 * This is the counterpart to openDatabase(). It is called after a sync. Sub-classes may
 * use it to write out data, release resources and so on. On success, it shall return true, on
 * error, false.
 *
 * The default implementation does nothing.
 *
 * @sa openDatabase()
 */
bool SyncStateDatabase::closeDatabase()
{
    setOpen(false);
    return true;
}

/**
 * @brief Get if the database open.
 *
 * This returns true if the database previously has been opened successfully via openDatabase().
 */
bool SyncStateDatabase::isOpen() const
{
    Q_D(const SyncStateDatabase);
    return d->open;
}

/**
 * @brief Iterate over the entries in the database.
 *
 * This is a utility method which calls the @p callback for all nodes found in the database starting
 * at the given root @p path.
 *
 * The function returns true on success or false if - during iteration - an error occurred.
 */
bool SyncStateDatabase::iterate(std::function<void(const SyncStateEntry&)> callback,
                                const QString& path)
{
    QQueue<QString> queue;
    queue.enqueue(path);

    {
        auto entry = getEntry(path);
        if (callback && entry.isValid()) {
            callback(entry);
        }
    }

    while (!queue.isEmpty()) {
        bool ok;
        auto folderPath = queue.dequeue();
        auto entries = findEntries(folderPath, &ok);
        if (!ok) {
            return false;
        }
        for (const auto& entry : qAsConst(entries)) {
            if (entry.isValid()) {
                if (callback) {
                    callback(entry);
                }
                queue.enqueue(entry.path());
            }
        }
    }
    return true;
}

/**
 * @brief Constructor.
 */
SyncStateDatabase::SyncStateDatabase(SyncStateDatabasePrivate* d, QObject* parent)
    : QObject(parent), d_ptr(d)
{
}

void SyncStateDatabase::setOpen(bool open)
{
    Q_D(SyncStateDatabase);
    d->open = open;
}

} // namespace SynqClient
