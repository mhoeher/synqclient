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

#include "../inc/webdavdownloadfilejob.h"

#include "abstractwebdavjobprivate.h"
#include "webdavdownloadfilejobprivate.h"

namespace SynqClient {

WebDAVDownloadFileJob::WebDAVDownloadFileJob(QObject* parent)
    : DownloadFileJob(new WebDAVDownloadFileJobPrivate(this), parent)
{
}

WebDAVDownloadFileJob::~WebDAVDownloadFileJob() {}

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
    d_ptr2->prepareNetworkRequest(req);
    req.setUrl(url);
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
        connect(reply, &QNetworkReply::readyRead, [=]() {
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

void WebDAVDownloadFileJob::stop()
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

WebDAVDownloadFileJob::WebDAVDownloadFileJob(WebDAVDownloadFileJobPrivate* d, QObject* parent)
    : DownloadFileJob(d, parent)
{
}

} // namespace SynqClient
