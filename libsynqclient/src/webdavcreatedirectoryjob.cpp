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

#include "SynqClient/webdavcreatedirectoryjob.h"

#include "webdavcreatedirectoryjobprivate.h"
#include "abstractwebdavjobprivate.h"

namespace SynqClient {

/**
 * @class WebDAVCreateDirectoryJob
 * @brief Implementation of the CreateDirectoryJob for WebDAV.
 */

/**
 * @brief Constructor.
 */
WebDAVCreateDirectoryJob::WebDAVCreateDirectoryJob(QObject* parent)
    : CreateDirectoryJob(new WebDAVCreateDirectoryJobPrivate(this), parent), AbstractWebDAVJob()
{
}

/**
 * @brief Destructor.
 */
WebDAVCreateDirectoryJob::~WebDAVCreateDirectoryJob() {}

/**
 * @brief Implementation of AbstractJob::start().
 */
void WebDAVCreateDirectoryJob::start()
{
    Q_D(WebDAVCreateDirectoryJob);
    d->state = JobState::Running;

    // Check for missing parameters:
    d->checkParameters();
    if (error() != JobError::NoError) {
        finishLater();
        return;
    }

    auto url = d_ptr2->urlFromPath(d->path);
    if (!d_ptr2->nextUrl.isValid()) {
        // This is the initial try to create the directory (after redirection).
        // Make sure that the URL ends with a slash, otherwise, servers
        // might return a Content Conflict error (see
        // https://gitlab.com/rpdev/synqclient/-/issues/14 for details):
        auto urlPath = url.path();
        if (!urlPath.endsWith("/")) {
            urlPath.append("/");
            url.setPath(urlPath);
        }
    }
    QNetworkRequest req;
    d_ptr2->prepareNetworkRequest(req, this);
    req.setUrl(url);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::ManualRedirectPolicy); // WA for QTBUG-92909, handle redirects
                                                             // manually in client code
    auto reply = networkAccessManager()->sendCustomRequest(req, d_ptr2->MKCOL);
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
void WebDAVCreateDirectoryJob::stop()
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
WebDAVCreateDirectoryJob::WebDAVCreateDirectoryJob(WebDAVCreateDirectoryJobPrivate* d,
                                                   QObject* parent)
    : CreateDirectoryJob(d, parent), AbstractWebDAVJob()
{
}

} // namespace SynqClient
