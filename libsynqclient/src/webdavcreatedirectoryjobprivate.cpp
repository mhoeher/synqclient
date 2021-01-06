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

#include "abstractwebdavjobprivate.h"
#include "webdavcreatedirectoryjobprivate.h"
#include "abstractwebdavjobprivate.h"

namespace SynqClient {

WebDAVCreateDirectoryJobPrivate::WebDAVCreateDirectoryJobPrivate(WebDAVCreateDirectoryJob* q)
    : CreateDirectoryJobPrivate(q)
{
}

void WebDAVCreateDirectoryJobPrivate::checkParameters()
{
    Q_Q(WebDAVCreateDirectoryJob);
    if (!q->networkAccessManager()) {
        q->setError(JobError::MissingParameter, "No QNetworkAccessManager set");
    }
    if (!q->url().isValid()) {
        q->setError(JobError::MissingParameter, "No URL set");
    }
    if (q->path().isEmpty()) {
        q->setError(JobError::MissingParameter, "No path set");
    }
}

void WebDAVCreateDirectoryJobPrivate::handleRequestFinished()
{
    Q_Q(WebDAVCreateDirectoryJob);
    auto reply = q->d_ptr2->reply;
    q->d_ptr2->reply = nullptr;
    if (reply) {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            auto code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            switch (code) {
            case AbstractWebDAVJobPrivate::HTTPNotAllowed:
                q->setError(JobError::FolderExists, tr("The remote folder already exists"));
                break;
            default:
                q->setError(q->fromNetworkError(*reply), reply->errorString());
                break;
            }
            q->finishLater();
        } else if (q->d_ptr2->shouldFollowUnhandledRedirect()) {
            // Encountered redirect not handled by Qt, follow:
            q->start();
            return;
        } else {
            auto code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            if (code.toInt() == q->d_ptr2->WebDAVCreated) {
                // Everything fine - nothing else to do.
            } else {
                q->setError(
                        JobError::InvalidResponse,
                        QString("Creating directory failed with error code: %1").arg(code.toInt()));
            }
            q->finishLater();
        }
    }
}

} // namespace SynqClient
