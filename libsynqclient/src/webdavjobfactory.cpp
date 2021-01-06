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

#include "../inc/webdavjobfactory.h"

#include "webdavjobfactoryprivate.h"

#include "webdavcreatedirectoryjob.h"
#include "webdavdeletejob.h"
#include "webdavdownloadfilejob.h"
#include "webdavuploadfilejob.h"
#include "webdavgetfileinfojob.h"
#include "webdavlistfilesjob.h"

namespace SynqClient {

/**
 * @class WebDAVJobFactory
 * @brief Create jobs to talk to a WebDAV server.
 *
 * This factory is used to create jobs which can be used to talk with a
 * concrete WebDAV server. The factory needs to be configured: At least a
 * networkAccessManager() must be set as well as the base url() of the server
 * to talk to.
 */

/**
 * @brief Constructor.
 */
WebDAVJobFactory::WebDAVJobFactory(QObject* parent)
    : AbstractJobFactory(new WebDAVJobFactoryPrivate(this), parent)
{
}

/**
 * @brief Destructor.
 */
WebDAVJobFactory::~WebDAVJobFactory() {}

/**
 * @brief The network access manager used by jobs created by the factory.
 */
QNetworkAccessManager* WebDAVJobFactory::networkAccessManager() const
{
    Q_D(const WebDAVJobFactory);
    return d->networkAccessManager;
}

/**
 * @brief Set the network access manager to be used by created jobs.
 */
void WebDAVJobFactory::setNetworkAccessManager(QNetworkAccessManager* networkAccessManager)
{
    Q_D(WebDAVJobFactory);
    d->networkAccessManager = networkAccessManager;
}

/**
 * @brief The base URL of the server to connect to.
 */
QUrl WebDAVJobFactory::url() const
{
    Q_D(const WebDAVJobFactory);
    return d->url;
}

/**
 * @brief Set the base URL of the server to connect to.
 */
void WebDAVJobFactory::setUrl(const QUrl& url)
{
    Q_D(WebDAVJobFactory);
    d->url = url;
}

/**
 * @brief The default user agent used by jobs created by the factory.
 */
QString WebDAVJobFactory::userAgent() const
{
    Q_D(const WebDAVJobFactory);
    return d->userAgent;
}

/**
 * @brief Set the default user agent for jobs created by the factory.
 */
void WebDAVJobFactory::setUserAgent(const QString& userAgent)
{
    Q_D(WebDAVJobFactory);
    d->userAgent = userAgent;
}

/**
 * @brief The type of WebDAV server to connect to.
 */
WebDAVServerType WebDAVJobFactory::serverType() const
{
    Q_D(const WebDAVJobFactory);
    return d->serverType;
}

/**
 * @brief Set the type of WebDAV server to connect to.
 */
void WebDAVJobFactory::setServerType(WebDAVServerType serverType)
{
    Q_D(WebDAVJobFactory);
    d->serverType = serverType;
}

/**
 * @brief Constructor.
 */
WebDAVJobFactory::WebDAVJobFactory(WebDAVJobFactoryPrivate* d, QObject* parent)
    : AbstractJobFactory(d, parent)
{
}

/**
 * @brief Create a WebDAV job.
 *
 * This function implements the appropriate API from the AbstractJobFactory
 * class. Depending on the requested @p type, it returns the appropriate
 * concrete WebDAV jobs.
 */
AbstractJob* WebDAVJobFactory::createJob(JobType type, QObject* parent)
{
    Q_D(WebDAVJobFactory);
    switch (type) {
    case JobType::CreateDirectory:
        return d->createJob<WebDAVCreateDirectoryJob>(parent);
    case JobType::DeleteResource:
        return d->createJob<WebDAVDeleteJob>(parent);
    case JobType::DownloadFile:
        return d->createJob<WebDAVDownloadFileJob>(parent);
    case JobType::UploadFile:
        return d->createJob<WebDAVUploadFileJob>(parent);
    case JobType::GetFileInfo:
        return d->createJob<WebDAVGetFileInfoJob>(parent);
    case JobType::ListFiles:
        return d->createJob<WebDAVListFilesJob>(parent);
    default:
        return nullptr;
    }
}

} // namespace SynqClient
