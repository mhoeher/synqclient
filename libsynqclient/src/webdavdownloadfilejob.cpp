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

#include "SynqClient/webdavdownloadfilejob.h"

#include "abstractwebdavjobprivate.h"
#include "webdavdownloadfilejobprivate.h"

namespace SynqClient {

/**
 * @class WebDAVDownloadFileJob
 * @brief Implementation of the DownloadFileJob for WebDAV.
 */

/**
 * @brief Constructor.
 */
WebDAVDownloadFileJob::WebDAVDownloadFileJob(QObject* parent)
    : DownloadFileJob(new WebDAVDownloadFileJobPrivate(this), parent)
{
}

/**
 * @brief Destructor.
 */
WebDAVDownloadFileJob::~WebDAVDownloadFileJob() {}

/**
 * @brief Implementation of AbstractJob::start().
 */
void WebDAVDownloadFileJob::start()
{
    Q_D(WebDAVDownloadFileJob);
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
    // Turn server side compression off. This is required because some servers tend to modify
    // etags. In that case, we get different etags via the list method and after downloading.
    req.setRawHeader("Accept-Encoding", "identity");
    if (d->downloadDevice) {
        if (d->downloadDevice != d->output) {
            delete d->downloadDevice;
            d->downloadDevice = nullptr;
        }
    }
    d->downloadDevice = getDownloadDevice();

    if (error() != JobError::NoError) {
        finishLater();
        return;
    }

    auto downloadDevice = d->downloadDevice;
    d->downloadDevice->seek(0);
    req.setHeader(QNetworkRequest::ContentTypeHeader, d_ptr2->OctetStreamEncoding);
    auto reply = networkAccessManager()->get(req);
    if (reply) {
        reply->setParent(this);
        connect(reply, &QNetworkReply::readyRead, this, [=]() {
            if (downloadDevice) {
                downloadDevice->write(reply->readAll());
            }
        });
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
void WebDAVDownloadFileJob::stop()
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
WebDAVDownloadFileJob::WebDAVDownloadFileJob(WebDAVDownloadFileJobPrivate* d, QObject* parent)
    : DownloadFileJob(d, parent)
{
}

} // namespace SynqClient
