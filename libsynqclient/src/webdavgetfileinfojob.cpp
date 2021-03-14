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

#include "SynqClient/webdavgetfileinfojob.h"

#include <QNetworkRequest>

#include "abstractwebdavjobprivate.h"
#include "webdavgetfileinfojobprivate.h"

namespace SynqClient {

/**
 * @class WebDAVGetFileInfoJob
 * @brief Implementation of the GetFileInfoJob for WebDAV.
 */

/**
 * @brief Constructor.
 */
WebDAVGetFileInfoJob::WebDAVGetFileInfoJob(QObject* parent)
    : GetFileInfoJob(new WebDAVGetFileInfoJobPrivate(this), parent), AbstractWebDAVJob()
{
}

/**
 * @brief Destructor..
 */
WebDAVGetFileInfoJob::~WebDAVGetFileInfoJob() {}

/**
 * @brief Constructor.
 */
WebDAVGetFileInfoJob::WebDAVGetFileInfoJob(WebDAVGetFileInfoJobPrivate* d, QObject* parent)
    : GetFileInfoJob(d, parent), AbstractWebDAVJob()
{
}

/**
 * @brief Implementation of AbstractJob::start().
 */
void WebDAVGetFileInfoJob::start()
{
    Q_D(WebDAVGetFileInfoJob);
    d->state = JobState::Running;

    // Check for missing parameters:
    d->checkParameters();
    if (error() != JobError::NoError) {
        finishLater();
        return;
    }

    auto url = d_ptr2->urlFromPath(d->path);
    QNetworkRequest req;
    d_ptr2->prepareNetworkRequest(req);
    d_ptr2->disableCaching(req);
    req.setUrl(url);
    req.setRawHeader("Depth", "0");
    req.setHeader(QNetworkRequest::ContentLengthHeader,
                  AbstractWebDAVJobPrivate::PropFindRequestData.size());
    req.setHeader(QNetworkRequest::ContentTypeHeader, d_ptr2->DefaultEncoding);
    auto reply = networkAccessManager()->sendCustomRequest(
            req, d_ptr2->PROPFIND, AbstractWebDAVJobPrivate::PropFindRequestData);
    if (reply) {
        reply->setParent(this);
        connect(reply, &QNetworkReply::finished, [=]() { d->handleRequestFinished(); });
        d_ptr2->reply = reply;
    } else {
        setError(JobError::InvalidResponse, "Received null network reply");
        finishLater();
    }
}

/**
 * @brief Implementation of AbstractJob::stop().
 */
void WebDAVGetFileInfoJob::stop()
{
    auto reply = d_ptr2->reply;
    if (reply) {
        reply->abort();
        delete reply;
        d_ptr2->reply = nullptr;
    }
    setError(JobError::Stopped, "The job has been stopped");
    finishLater();
}

} // namespace SynqClient
