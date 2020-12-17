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

#include "../inc/syncstateentry.h"

#include <QDir>

#include "syncstateentryprivate.h"

namespace SynqClient {

/**
 * @class SyncStateEntry
 * @brief Holds state information for a single file or folder.
 *
 * This class holds the path of a file or folder as well as additional local and
 * remote status information, which is used in each sync run to decide if the item
 * needs to be updated.
 */

/**
 * @brief Constructor.
 */
SyncStateEntry::SyncStateEntry() : d(new SyncStateEntryPrivate) {}

/**
 * @brief Copy constructor.
 */
SyncStateEntry::SyncStateEntry(const SyncStateEntry& other) : d(other.d) {}

/**
 * @brief Constructor.
 *
 * Creates a new entry with the given @p path, @p modificationTime and @p syncProperty. The entry
 * will be valid (i.e. isValid() returns true).
 */
SyncStateEntry::SyncStateEntry(const QString& path, const QDateTime& modificationTime,
                               const QString& syncProperty)
    : d(new SyncStateEntryPrivate)
{
    d->modificationTime = modificationTime;
    d->path = path;
    d->syncProperty = syncProperty;
    d->valid = true;
}

/**
 * @brief Destructor.
 */
SyncStateEntry::~SyncStateEntry() {}

/**
 * @brief Assignment operator.
 */
SyncStateEntry& SyncStateEntry::operator=(const SyncStateEntry& other)
{
    d = other.d;
    return *this;
}

/**
 * @brief Used to indicate if the entry is valid.
 *
 * This property can be used to indicate if the entry is valid. By default, an entry
 * is initialized with a value of `false`. APIs that look up entries and return them shall
 * set it to true on returned entries.
 */
bool SyncStateEntry::isValid() const
{
    return d->valid;
}

/**
 * @brief Set the valid property.
 */
void SyncStateEntry::setValid(bool valid)
{
    d->valid = valid;
}

/**
 * @brief The file path.
 *
 * This is the path of the entry in a synchronized directory. The path is of the form
 * `/parent/sub/othersub/file.txt`, i.e.:
 *
 * - It is stored absolute (and implicitly assumed to be relative to the root of the
 *   local/remote directory to be synced).
 * - It uses forward slashes on all platforms.
 * - The top level folder is represented as `/`.
 * - For all folders except the top level one, the path must not end with a slash.
 *
 * If a path is not in that form, it will be converted accordingly by this class upon
 * setting the path.
 */
QString SyncStateEntry::path() const
{
    return d->path;
}

/**
 * @brief Set the path of the file or folder.
 */
void SyncStateEntry::setPath(const QString& path)
{
    d->path = makePath(path);
}

/**
 * @brief Holds the last modification date and time.
 *
 * This property holds the date and time of the last modification that was done locally.
 * This is required to detect changes to the file that happen in the local directory.
 */
QDateTime SyncStateEntry::modificationTime() const
{
    return d->modificationTime;
}

/**
 * @brief Set the date and time of the last local modification.
 */
void SyncStateEntry::setModificationTime(const QDateTime& modificationTime)
{
    d->modificationTime = modificationTime;
}

/**
 * @brief The remote sync property.
 *
 * This property holds the value of the remote *sync property* (converted to a string). The
 * sync property must be reported by jobs retrieving the remote file information. It is
 * supposed to be a property which can be used to detect remote changes. The concrete
 * value and form depends on the backend used. For example, for WebDAV, this property
 * could be the *etags* reported by the server for the files and folders.
 */
QString SyncStateEntry::syncProperty() const
{
    return d->syncProperty;
}

/**
 * @brief Set the remote sync property of the file or folder.
 */
void SyncStateEntry::setSyncProperty(const QString& syncProperty)
{
    d->syncProperty = syncProperty;
}

/**
 * @brief Convert a path to a sync entry path.
 *
 * This is a helper function which returns a path suitable to be used with the sync state
 * database API. Internally, this is used by setPath() to make sure paths set are stored in
 * a suitable format internally.
 *
 * This method can also be used by other parts to create suitable paths similar to the ones used
 * internally by this class.
 */
QString SyncStateEntry::makePath(const QString& path)
{
    auto p = path;
    if (!p.startsWith("/")) {
        p = "/" + p;
    }
    while (p.length() > 1 && p.endsWith("/")) {
        p.chop(1);
    }
    return QDir::cleanPath(p);
}

SyncStateEntry::SyncStateEntry(SyncStateEntryPrivate* d) : d(d) {}

} // namespace SynqClient
