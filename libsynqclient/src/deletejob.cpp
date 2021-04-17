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

#include "SynqClient/deletejob.h"

#include "deletejobprivate.h"

namespace SynqClient {

/**
 * @class DeleteJob
 * @brief Delete remote files or folders.
 *
 * This class is an abstract base for jobs to delete remote resources. The path() to the remote
 * resource to delete needs to be specified. In addition, a syncAttribute() can be set: In this
 * case, the deletion should - if possible - only be carried out if the remote version of the
 * resource did not change.
 *
 * If the remote resource is a folder, it is deleted recursively, i.e. all contained files and
 * sub-folders (and their contents in turn) will be removed.
 *
 * # Error Handling
 *
 * Besides the usual error handling, the following error codes are used to indicate non-fatal
 * errors:
 *
 * - JobError::SyncAttributeMismatch: This error indicates that the remote resource was not deleted
 * due to the sync attribute does not match, i.e. there was a *lost update*.
 * - JobError::ResourceNotFound: This error indicates that the remote resource does not/no longer
 * exist.
 */

/**
 * @brief Constructor.
 */
DeleteJob::DeleteJob(QObject* parent) : AbstractJob(new DeleteJobPrivate(this), parent) {}

/**
 * @brief Destructor.
 */
DeleteJob::~DeleteJob() {}

/**
 * @brief The path to the remote file or folder to delete.
 */
QString DeleteJob::path() const
{
    Q_D(const DeleteJob);
    return d->path;
}

/**
 * @brief Set the path to the remote file or folder to be deleted.
 */
void DeleteJob::setPath(const QString& path)
{
    Q_D(DeleteJob);
    d->path = path;
}

/**
 * @brief Delete only if the remote file's sync attribute matches.
 *
 * If this property is set to a valid (i.e. non-null) sync attribute,
 * then the request shall only succeed if the remote file's sync attribute matches
 * the set one.
 *
 * @note This might or might not work, depending on the concrete protocol in
 * question. If a backened does not support conditional deletes, this attribute will be ignored.
 *
 * @warning This might not work when deleting folders.
 */
QVariant DeleteJob::syncAttribute() const
{
    Q_D(const DeleteJob);
    return d->syncAttribute;
}

/**
 * @brief Set the @p syncAttribute we expect on the remote file.
 */
void DeleteJob::setSyncAttribute(const QVariant& syncAttribute)
{
    Q_D(DeleteJob);
    d->syncAttribute = syncAttribute;
}

/**
 * @brief Constructor.
 */
DeleteJob::DeleteJob(DeleteJobPrivate* d, QObject* parent) : AbstractJob(d, parent) {}

} // namespace SynqClient
