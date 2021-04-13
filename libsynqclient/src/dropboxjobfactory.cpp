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

#include "../inc/SynqClient/dropboxjobfactory.h"

#include "dropboxjobfactoryprivate.h"
#include "SynqClient/DropboxCreateDirectoryJob"
#include "SynqClient/DropboxDeleteJob"
#include "SynqClient/DropboxDownloadFileJob"
#include "SynqClient/DropboxGetFileInfoJob"
#include "SynqClient/DropboxListFilesJob"
#include "SynqClient/DropboxUploadFileJob"

namespace SynqClient {

/**
 * @brief Constructor.
 */
DropboxJobFactory::DropboxJobFactory(QObject* parent)
    : AbstractJobFactory(new DropboxJobFactoryPrivate(this), parent)
{
}

/**
 * @brief Destructor.
 */
DropboxJobFactory::~DropboxJobFactory() {}

/**
 * @brief The network access manager to be used by created jobs.
 *
 * This property holds the QNetworkAccessManager that all created jobs of this factory will be using
 * by default.
 */
QNetworkAccessManager* DropboxJobFactory::networkAccessManager() const
{
    Q_D(const DropboxJobFactory);
    return d->networkAccessManager;
}

/**
 * @brief Set the @p networkAccessManager to be used by jobs created by this factory.
 */
void DropboxJobFactory::setNetworkAccessManager(QNetworkAccessManager* networkAccessManager)
{
    Q_D(DropboxJobFactory);
    d->networkAccessManager = networkAccessManager;
}

/**
 * @brief The user agent used by the created jobs.
 */
QString DropboxJobFactory::userAgent() const
{
    Q_D(const DropboxJobFactory);
    return d->userAgent;
}

/**
 * @brief Set the @p userAgent to be used by the created jobs.
 */
void DropboxJobFactory::setUserAgent(const QString& userAgent)
{
    Q_D(DropboxJobFactory);
    d->userAgent = userAgent;
}

/**
 * @brief The OAuth2 bearer token to be used to authenticate against the Dropbox API.
 */
QString DropboxJobFactory::token() const
{
    Q_D(const DropboxJobFactory);
    return d->token;
}

/**
 * @brief Set the OAuth2 bearer @p token to be used to authenticate against the Dropbox API.
 */
void DropboxJobFactory::setToken(const QString& token)
{
    Q_D(DropboxJobFactory);
    d->token = token;
}

/**
 * @brief Constructor.
 */
DropboxJobFactory::DropboxJobFactory(DropboxJobFactoryPrivate* d, QObject* parent)
    : AbstractJobFactory(d, parent)
{
}

/**
 * @brief Implementation of SynqClient::AbstractJobFactory::createJob().
 */
AbstractJob* DropboxJobFactory::createJob(JobType type, QObject* parent)
{
    Q_D(DropboxJobFactory);
    switch (type) {
    case JobType::CreateDirectory:
        return d->createJob<DropboxCreateDirectoryJob>(parent);
    case JobType::DeleteResource:
        return d->createJob<DropboxDeleteJob>(parent);
    case JobType::DownloadFile:
        return d->createJob<DropboxDownloadFileJob>(parent);
    case JobType::UploadFile:
        return d->createJob<DropboxUploadFileJob>(parent);
    case JobType::GetFileInfo:
        return d->createJob<DropboxGetFileInfoJob>(parent);
    case JobType::ListFiles:
        return d->createJob<DropboxListFilesJob>(parent);
    default:
        return nullptr;
    }
}

} // namespace SynqClient
