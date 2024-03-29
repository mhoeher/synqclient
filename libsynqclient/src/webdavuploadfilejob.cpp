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

#include "SynqClient/webdavuploadfilejob.h"

#include <QNetworkRequest>

#include "abstractwebdavjobprivate.h"
#include "webdavuploadfilejobprivate.h"

namespace SynqClient {

/**
 * @class WebDAVUploadFileJob
 * @brief Implementation of the UploadFileJob for WebDAV.
 */

/**
 * @brief Constructor.
 */
WebDAVUploadFileJob::WebDAVUploadFileJob(QObject* parent)
    : UploadFileJob(new WebDAVUploadFileJobPrivate(this), parent), AbstractWebDAVJob()
{
}

/**
 * @brief Destructor.
 */
WebDAVUploadFileJob::~WebDAVUploadFileJob() {}

/**
 * @brief Implementation of AbstractJob::start().
 */
void WebDAVUploadFileJob::start()
{
    Q_D(WebDAVUploadFileJob);
    d->state = JobState::Running;

    // Check for missing parameters:
    d->checkParameters();
    if (error() != JobError::NoError) {
        finishLater();
        return;
    }

    auto url = d_ptr2->urlFromPath(d->remoteFilename);
    QNetworkRequest req;
    d_ptr2->prepareNetworkRequest(req, this);
    d_ptr2->disableCaching(req);
    req.setUrl(url);
    if (!d->uploadDevice) {
        d->uploadDevice = getUploadDevice();
        if (error() != JobError::NoError) {
            finishLater();
            return;
        }
    }
    auto uploadDevice = d->uploadDevice;
    uploadDevice->seek(0);
    req.setHeader(QNetworkRequest::ContentLengthHeader, uploadDevice->size());
    req.setHeader(QNetworkRequest::ContentTypeHeader, d_ptr2->OctetStreamEncoding);
    auto etag = syncAttribute();
    if (etag.isValid() && !etag.toString().isEmpty()) {
        req.setHeader(QNetworkRequest::IfMatchHeader, etag.toString());
    }
    auto reply = networkAccessManager()->put(req, uploadDevice.data());
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
void WebDAVUploadFileJob::stop()
{
    if (state() == JobState::Running) {
        auto reply = d_ptr2->reply;
        if (reply) {
            reply->abort();
            delete reply;
            d_ptr2->reply = nullptr;
        }
        setError(JobError::Stopped, "The job has been stopped");
        finishLater();
    }
}

/**
 * @brief Constructor.
 */
WebDAVUploadFileJob::WebDAVUploadFileJob(WebDAVUploadFileJobPrivate* d, QObject* parent)
    : UploadFileJob(d, parent), AbstractWebDAVJob()
{
}

} // namespace SynqClient
