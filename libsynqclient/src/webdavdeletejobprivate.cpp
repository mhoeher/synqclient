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

#include "webdavdeletejobprivate.h"

#include "abstractwebdavjobprivate.h"

namespace SynqClient {

WebDAVDeleteJobPrivate::WebDAVDeleteJobPrivate(WebDAVDeleteJob* q) : DeleteJobPrivate(q) {}

void WebDAVDeleteJobPrivate::checkParameters()
{
    Q_Q(WebDAVDeleteJob);
    if (!q->networkAccessManager()) {
        q->setError(JobError::MissingParameter, "No QNetworkAccessManager set");
    }
    if (!q->url().isValid()) {
        q->setError(JobError::MissingParameter, "No URL set");
    }
}

void WebDAVDeleteJobPrivate::handleRequestFinished()
{
    Q_Q(WebDAVDeleteJob);
    auto reply = q->d_ptr2->reply;
    q->d_ptr2->reply = nullptr;
    if (reply) {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            q->setError(q->fromNetworkError(*reply), reply->errorString());
            q->finishLater();
        } else if (q->d_ptr2->shouldFollowUnhandledRedirect()) {
            // Encountered redirect not handled by Qt, follow:
            q->start();
            return;
        } else {
            auto code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            if (code.toInt() == q->d_ptr2->HTTPOkay || code.toInt() == q->d_ptr2->HTTPNoContent) {
                // All done!
            } else if (code.toInt() == q->d_ptr2->HTTPForbidden) {
                // The user is not allowed to delete the file/folder:
                q->setError(JobError::Forbidden,
                            QString("Delete operation is forbidden for user on that resource"));
            } else {
                q->setError(JobError::InvalidResponse,
                            QString("Received invalid response from server: %1").arg(code.toInt()));
            }
            q->finishLater();
        }
    }
}

} // namespace SynqClient
