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

#include "SynqClient/webdavlistfilesjob.h"

#include "abstractwebdavjobprivate.h"
#include "webdavlistfilesjobprivate.h"

namespace SynqClient {

/**
 * @class WebDAVListFilesJob
 * @brief Implementation of the ListFilesJob for WebDAV.
 */

/**
 * @brief Constructor.
 */
WebDAVListFilesJob::WebDAVListFilesJob(QObject* parent)
    : ListFilesJob(new WebDAVListFilesJobPrivate(this), parent), AbstractWebDAVJob()
{
}

/**
 * @brief Destructor.
 */
WebDAVListFilesJob::~WebDAVListFilesJob() {}

/**
 * @brief Implementation of AbstractJob::start().
 */
void WebDAVListFilesJob::start()
{
    Q_D(WebDAVListFilesJob);
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
        // Make sure that the URL ends with a slash (see
        // https://gitlab.com/rpdev/synqclient/-/issues/14 for details). Otherwise
        // we likely might get redirects:
        auto urlPath = url.path();
        if (!urlPath.endsWith("/")) {
            urlPath.append("/");
            url.setPath(urlPath);
        }
    }
    QNetworkRequest req;
    d_ptr2->prepareNetworkRequest(req);
    d_ptr2->disableCaching(req);
    req.setUrl(url);
    req.setRawHeader("Depth", "1");
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
void WebDAVListFilesJob::stop()
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

/**
 * @brief Constructor.
 */
WebDAVListFilesJob::WebDAVListFilesJob(WebDAVListFilesJobPrivate* d, QObject* parent)
    : ListFilesJob(d, parent), AbstractWebDAVJob()
{
}

} // namespace SynqClient
