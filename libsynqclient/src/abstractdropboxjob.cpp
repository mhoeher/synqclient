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

#include "../inc/SynqClient/abstractdropboxjob.h"

#include "abstractdropboxjobprivate.h"

namespace SynqClient {

/**
 * @brief The key used to store retrieved file or folder metadata in a FileInfo object.
 *
 * This key is used to store the original JSON metadata of a file or folder as retrieved by
 * the Dropbox API in a FileInfo object (via FileInfo::setCustomProperty()).
 */
const QString AbstractDropboxJob::DropboxFileInfoKey = "Dropbox";

/**
 * @class AbstractDropboxJob
 * @brief Shared interface for all Dropbox related jobs.
 *
 * This class provides the shared interface that all Dropbox jobs require.
 */

/**
 * @brief Constructor.
 */
AbstractDropboxJob::AbstractDropboxJob() : d_ptr2(new AbstractDropboxJobPrivate(this)) {}

/**
 * @brief Destructor.
 */
AbstractDropboxJob::~AbstractDropboxJob() {}

/**
 * @brief The network access manager used by the job.
 */
QNetworkAccessManager* AbstractDropboxJob::networkAccessManager() const
{
    Q_D(const AbstractDropboxJob);
    return d->networkAccessManager;
}

/**
 * @brief Set the @p networkAccessManager to be used by the job.
 */
void AbstractDropboxJob::setNetworkAccessManager(QNetworkAccessManager* networkAccessManager)
{
    Q_D(AbstractDropboxJob);
    d->networkAccessManager = networkAccessManager;
}

/**
 * @brief The user agent used when making network requests.
 */
QString AbstractDropboxJob::userAgent() const
{
    Q_D(const AbstractDropboxJob);
    return d->userAgent;
}

/**
 * @brief Set the @p userAgent to be used when making network requests.
 */
void AbstractDropboxJob::setUserAgent(const QString& userAgent)
{
    Q_D(AbstractDropboxJob);
    d->userAgent = userAgent;
}

/**
 * @brief The OAuth2 bearer token used to authenticate against the Dropbox API.
 */
QString AbstractDropboxJob::token() const
{
    Q_D(const AbstractDropboxJob);
    return d->token;
}

/**
 * @brief Set the OAuth2 bearer @p token to be used to authenticate against the Dropbox API.
 */
void AbstractDropboxJob::setToken(const QString& token)
{
    Q_D(AbstractDropboxJob);
    d->token = token;
}

/**
 * @brief Constructor.
 */
AbstractDropboxJob::AbstractDropboxJob(AbstractDropboxJobPrivate* d) : d_ptr2(d) {}

} // namespace SynqClient
