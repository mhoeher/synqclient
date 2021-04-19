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

#include "SynqClient/listfilesjob.h"

#include "listfilesjobprivate.h"

namespace SynqClient {

/**
 * @class ListFilesJob
 * @brief List the contents of a remote folder.
 *
 * This is an abstract base class for jobs which are used to list the contents of a remote folder.
 * If the job succeeds, it holds a list of entries() (consisting of files and sub-folders) of the
 * remote folder. Additionally, information about the folder itself can be retrieved by using the
 * folder() method.
 *
 * @note If the path() configured points to a remote file, the job still should succeed. In this
 * case, information about the file is found in the file information returned by folder() and
 * entries() shall return an empty list. If a concrete implementation does not support this, it
 * shall fail with a JobError::RemoteResourceIsNotAFolder error.
 */

/**
 * @brief Constructor.
 */
ListFilesJob::ListFilesJob(QObject* parent) : AbstractJob(new ListFilesJobPrivate(this), parent) {}

/**
 * @brief Destructor.
 */
ListFilesJob::~ListFilesJob() {}

/**
 * @brief The path of the folder to list entries for.
 */
QString ListFilesJob::path() const
{
    Q_D(const ListFilesJob);
    return d->path;
}

/**
 * @brief Set the path to the folder for which to list entries.
 */
void ListFilesJob::setPath(const QString& path)
{
    Q_D(ListFilesJob);
    d->path = path;
}

/**
 * @brief File information about entries inside the folder.
 *
 * This property holds the file information list of files and sub-folders contained in the listed
 * folder.
 */
FileInfos ListFilesJob::entries() const
{
    Q_D(const ListFilesJob);
    return d->entries;
}

/**
 * @brief File info for the folder which has been listed.
 *
 * This property holds information about the folder that has been listed.
 */
FileInfo ListFilesJob::folder() const
{
    Q_D(const ListFilesJob);
    return d->folder;
}

/**
 * @brief Shall files and folders be listed recursively.
 *
 * If this property is set to true, listing entries within a folder is done recursively. The default
 * is false.
 *
 * @note Not all implementations support recursive listing.
 */
bool ListFilesJob::recursive() const
{
    Q_D(const ListFilesJob);
    return d->recursive;
}

/**
 * @brief Set if files and folder shall be listed recursively.
 */
void ListFilesJob::setRecursive(bool recursive)
{
    Q_D(ListFilesJob);
    d->recursive = recursive;
}

/**
 * @brief A cursor to continue the folder listing later on.
 *
 * This property holds a cursor - i.e. a token which is backend specific - which allows to query
 * the folder later on for any changes. After listing a specific folder, one can read the cursor and
 * - later on - create a new ListFilesJob object, setting the cursor on it. The new job will then
 * only list changes compared to the last list operation.
 *
 * @note Not all implementations support cursors. Refer to the implementation of the concrete class
 * to learn if it has this capability or not.
 */
QString ListFilesJob::cursor() const
{
    Q_D(const ListFilesJob);
    return d->cursor;
}

void ListFilesJob::setCursor(const QString& cursor)
{
    Q_D(ListFilesJob);
    d->cursor = cursor;
}

/**
 * @brief Constructor.
 */
ListFilesJob::ListFilesJob(ListFilesJobPrivate* d, QObject* parent) : AbstractJob(d, parent) {}

/**
 * @brief Set the file information of entries inside the folder.
 *
 * Concrete sub-classes shall use this method to set the file information about individual files and
 * sub-folders of the listed folder.
 */
void ListFilesJob::setEntries(const FileInfos& entries)
{
    Q_D(ListFilesJob);
    d->entries = entries;
}

/**
 * @brief Set folder file information.
 *
 * Concrete sub-classes shall use this method to set the file information of the listed folder.
 */
void ListFilesJob::setFolder(const FileInfo& folder)
{
    Q_D(ListFilesJob);
    d->folder = folder;
}

} // namespace SynqClient
