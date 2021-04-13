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

#include "../inc/SynqClient/dropboxuploadfilejob.h"

#include "dropboxuploadfilejobprivate.h"

namespace SynqClient {

/**
 * @class DropboxUploadFileJob
 * @brief Implementation of the UploadFileJob for Dropbox.
 */

/**
 * @brief Constructor.
 */
DropboxUploadFileJob::DropboxUploadFileJob(QObject* parent)
    : UploadFileJob(new DropboxUploadFileJobPrivate(this), parent), AbstractDropboxJob()
{
}

/**
 * @brief Destructor.
 */
DropboxUploadFileJob::~DropboxUploadFileJob() {}

/**
 * @brief Implementation of AbstractJob::start().
 */
void DropboxUploadFileJob::start()
{
    // TODO: Implement me
}

/**
 * @brief Implementation of AbstractJob::stop().
 */
void DropboxUploadFileJob::stop()
{
    // TODO: Implement me
}

/**
 * @brief Constructor.
 */
DropboxUploadFileJob::DropboxUploadFileJob(DropboxUploadFileJobPrivate* d, QObject* parent)
    : UploadFileJob(d, parent), AbstractDropboxJob()
{
}

} // namespace SynqClient
