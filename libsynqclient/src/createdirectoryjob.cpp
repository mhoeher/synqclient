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

#include "SynqClient/createdirectoryjob.h"

#include "createdirectoryjobprivate.h"

namespace SynqClient {

/**
 * @class CreateDirectoryJob
 * @brief Create a remote folder.
 *
 * This is an abstract base class for jobs which are used to create remote folders. The path to the
 * folder to be created is specified by calling setPath().
 *
 * Note that the parent folder of the configured path must exist, i.e. jobs of this type do not
 * create remote folders *recursively*.
 *
 * # Error Handling
 *
 * Besides the usual errors, the following codes are used to warn about non-fatal errors:
 *
 * - JobError::FolderExists: The remote folder already exists.
 */

/**
 * @brief Constructor.
 */
CreateDirectoryJob::CreateDirectoryJob(QObject* parent)
    : AbstractJob(new CreateDirectoryJobPrivate(this), parent)
{
}

/**
 * @brief Destructor.
 */
CreateDirectoryJob::~CreateDirectoryJob() {}

/**
 * @brief The path of the remote folder to be created.
 */
QString CreateDirectoryJob::path() const
{
    Q_D(const CreateDirectoryJob);
    return d->path;
}

/**
 * @brief Set the path to the remote folder to be created.
 */
void CreateDirectoryJob::setPath(const QString& path)
{
    Q_D(CreateDirectoryJob);
    d->path = path;
}

/**
 * @brief Constructor.
 */
CreateDirectoryJob::CreateDirectoryJob(CreateDirectoryJobPrivate* d, QObject* parent)
    : AbstractJob(d, parent)
{
}

} // namespace SynqClient
