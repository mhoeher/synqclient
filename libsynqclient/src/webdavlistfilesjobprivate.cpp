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

#include "webdavlistfilesjobprivate.h"

#include <QVariant>

#include "abstractwebdavjobprivate.h"

namespace SynqClient {

WebDAVListFilesJobPrivate::WebDAVListFilesJobPrivate(WebDAVListFilesJob* q) : ListFilesJobPrivate(q)
{
}

void WebDAVListFilesJobPrivate::checkParameters()
{
    Q_Q(WebDAVListFilesJob);
    if (!q->networkAccessManager()) {
        q->setError(JobError::MissingParameter, "No QNetworkAccessManager set");
    }
    if (!q->url().isValid()) {
        q->setError(JobError::MissingParameter, "No URL set");
    }
}

void WebDAVListFilesJobPrivate::handleRequestFinished()
{
    Q_Q(WebDAVListFilesJob);
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
            if (code.toInt() == q->d_ptr2->WebDAVMultiStatus) {
                bool ok;
                auto entryList = q->d_ptr2->parseEntryList(reply->url(), reply->readAll(), ok);
                if (!ok) {
                    q->setError(JobError::InvalidResponse,
                                "PROPFIND response from server is not valid");
                } else {
                    FileInfos entries;
                    for (const auto& entry : entryList) {
                        if (entry.name() != ".") {
                            entries << entry;
                        }
                    }
                    q->setEntries(entries);
                }
            } else {
                q->setError(JobError::InvalidResponse,
                            QString("Received invalid response from server: %1").arg(code.toInt()));
            }
            q->finishLater();
        }
    }
}

} // namespace SynqClient
