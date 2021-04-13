/*
 * Copyright 2021 Martin Hoeher <martin@rpdev.net>
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

#include "../inc/SynqClient/dropboxdownloadfilejob.h"

#include "dropboxdownloadfilejobprivate.h"

namespace SynqClient {

/**
 * @class DropboxDownloadFileJob
 * @brief Implementation of the DownloadFileJob for Dropbox.
 */

/**
 * @brief Constructor.
 */
DropboxDownloadFileJob::DropboxDownloadFileJob(QObject* parent)
    : DownloadFileJob(new DropboxDownloadFileJobPrivate(this), parent), AbstractDropboxJob()
{
}

/**
 * @brief Destructor.
 */
DropboxDownloadFileJob::~DropboxDownloadFileJob() {}

/**
 * @brief Implementation of AbstractJob::start().
 */
void DropboxDownloadFileJob::start()
{
    // TODO: Implement me
}

/**
 * @brief Implementation of AbstractJob::stop().
 */
void DropboxDownloadFileJob::stop()
{
    // TODO: Implement me
}

/**
 * @brief Destructor.
 */
DropboxDownloadFileJob::DropboxDownloadFileJob(DropboxDownloadFileJobPrivate* d, QObject* parent)
    : DownloadFileJob(d, parent), AbstractDropboxJob()
{
}

} // namespace SynqClient
