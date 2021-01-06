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

#include "../inc/abstractwebdavjob.h"

#include "abstractwebdavjobprivate.h"

namespace SynqClient {

/**
 * @class AbstractWebDAVJob
 * @brief Shared interface of all WebDAV related jobs.
 *
 * This class defines a common interface for all of the WebDAV jobs. For example, as all jobs
 * require a QNetWorkAccessManager, it is specified as a property in this class.
 *
 * # Generic WebDAV Access vs Specific Servers
 *
 * By default, WebDAV jobs will assume the WebDAV server (which is specified by the url()) to be a
 * generic, standards-compliant  WebDAV server. This is indicated by the serverType() attribute.
 *
 * On top, there is support for some specific WebDAV server implementations. This support can be
 * enabled by setting the serverType() to the concrete backend implementation. This might affect the
 * interpretation of certain attributes of the jobs. Refer to the WebDAVServerType enumeration to
 * learn about supported backends and how they affect the job behaviour.
 */

/**
 * @brief Constructor.
 */
AbstractWebDAVJob::AbstractWebDAVJob() : d_ptr2(new AbstractWebDAVJobPrivate(this)) {}

/**
 * @brief Destructor.
 */
AbstractWebDAVJob::~AbstractWebDAVJob() {}

/**
 * @brief The network access manager used to run network requests.
 */
QNetworkAccessManager* AbstractWebDAVJob::networkAccessManager() const
{
    Q_D(const AbstractWebDAVJob);
    return d->networkAccessManager;
}

/**
 * @brief Set the network request manager to be used by the job.
 *
 * @note The job does not take ownership of the object.
 */
void AbstractWebDAVJob::setNetworkAccessManager(QNetworkAccessManager* networkAccessManager)
{
    Q_D(AbstractWebDAVJob);
    d->networkAccessManager = networkAccessManager;
}

/**
 * @brief The base URL of the server.
 *
 * This is the base URL of the WebDAV server to talk to. Concrete WebDAV jobs then specify paths to
 * be relative to the base URL.
 *
 * @note The URL should contain the username and password to be used to authenticate against to the
 * WebDAV server. If no login information is provided via the URL, you might have to listen for
 * appropriate signals of the set networkAccessManager() to provide them instead.
 *
 * @sa serverType()
 */
QUrl AbstractWebDAVJob::url() const
{
    Q_D(const AbstractWebDAVJob);
    return d->url;
}

/**
 * @brief Set the base URL.
 */
void AbstractWebDAVJob::setUrl(const QUrl& url)
{
    Q_D(AbstractWebDAVJob);
    d->url = url;
}

/**
 * @brief The type of WebDAV server we are talking to.
 *
 * This can be used to specify which kind of WebDAV server we talk to.
 */
WebDAVServerType AbstractWebDAVJob::serverType() const
{
    Q_D(const AbstractWebDAVJob);
    return d->serverType;
}

/**
 * @brief Set the type of WebDAV server we are talking to.
 */
void AbstractWebDAVJob::setServerType(WebDAVServerType serverType)
{
    Q_D(AbstractWebDAVJob);
    d->serverType = serverType;
}

/**
 * @brief The user agent to be used when communicating with the WebDAV server.
 */
QString AbstractWebDAVJob::userAgent() const
{
    Q_D(const AbstractWebDAVJob);
    return d->userAgent;
}

/**
 * @brief Set the user agent to be used when talking to the WebDAV server.
 */
void AbstractWebDAVJob::setUserAgent(const QString& userAgent)
{
    Q_D(AbstractWebDAVJob);
    d->userAgent = userAgent;
}

/**
 * @brief Constructor.
 */
AbstractWebDAVJob::AbstractWebDAVJob(AbstractWebDAVJobPrivate* d) : d_ptr2(d) {}

} // namespace SynqClient
