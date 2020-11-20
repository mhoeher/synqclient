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

#include "../inc/webdavdeletejob.h"

#include <QNetworkRequest>

#include "abstractwebdavjobprivate.h"
#include "webdavdeletejobprivate.h"

namespace SynqClient {

WebDAVDeleteJob::WebDAVDeleteJob(QObject* parent)
    : DeleteJob(new WebDAVDeleteJobPrivate(this), parent), AbstractWebDAVJob()
{
}

WebDAVDeleteJob::~WebDAVDeleteJob() {}

void WebDAVDeleteJob::start()
{
    Q_D(WebDAVDeleteJob);
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
    req.setUrl(url);
    auto reply = networkAccessManager()->deleteResource(req);
    if (reply) {
        reply->setParent(this);
        connect(reply, &QNetworkReply::finished, [=]() { d->handleRequestFinished(); });
        d_ptr2->reply = reply;
    } else {
        setError(JobError::InvalidResponse, "Received null network reply");
        finishLater();
    }
}

void WebDAVDeleteJob::stop()
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

WebDAVDeleteJob::WebDAVDeleteJob(WebDAVDeleteJobPrivate* d, QObject* parent)
    : DeleteJob(d, parent), AbstractWebDAVJob()
{
}

} // namespace SynqClient