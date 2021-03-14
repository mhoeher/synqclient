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

#include "SynqClient/getfileinfojob.h"

#include "getfileinfojobprivate.h"

namespace SynqClient {

/**
 * @class GetFileInfoJob
 * @brief Get information about a remote file or folder.
 *
 * This class is an abstract base class for jobs which retrieve information about remote files or
 * folders. The job is configured with a path to a remote file or folder. Running it will cause the
 * job to retrieve meta information about the remote file. Once the job succeeded, you can query the
 * information by using the fileInfo() method.
 */

/**
 * @brief Constructor.
 */
GetFileInfoJob::GetFileInfoJob(QObject* parent)
    : AbstractJob(new GetFileInfoJobPrivate(this), parent)
{
}

/**
 * @brief Destructor.
 */
GetFileInfoJob::~GetFileInfoJob() {}

/**
 * @brief The path to the remote file or folder to get information for.
 */
QString GetFileInfoJob::path() const
{
    Q_D(const GetFileInfoJob);
    return d->path;
}

/**
 * @brief Set the path to the remote file or folder to get information for.
 */
void GetFileInfoJob::setPath(const QString& path)
{
    Q_D(GetFileInfoJob);
    d->path = path;
}

/**
 * @brief Holds the retrieved information about the remote file.
 */
FileInfo GetFileInfoJob::fileInfo() const
{
    Q_D(const GetFileInfoJob);
    return d->fileInfo;
}

/**
 * @brief Constructor.
 */
GetFileInfoJob::GetFileInfoJob(GetFileInfoJobPrivate* d, QObject* parent) : AbstractJob(d, parent)
{
}

/**
 * @brief Set file information.
 *
 * Concrete sub-classes shall use this method to set the file meta information for the remote file
 * or folder if the job succeeds.
 */
void GetFileInfoJob::setFileInfo(const FileInfo& fileInfo)
{
    Q_D(GetFileInfoJob);
    d->fileInfo = fileInfo;
}

} // namespace SynqClient
