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

#include "webdavlistfilesjobprivate.h"

#include <QTimer>
#include <QVariant>

#include "abstractwebdavjobprivate.h"

namespace SynqClient {

WebDAVListFilesJobPrivate::WebDAVListFilesJobPrivate(WebDAVListFilesJob* q)
    : ListFilesJobPrivate(q), retryWithoutTrailingSlash(false), retryWithDepthZero(false)
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
        if (q->d_ptr2->checkIfRequestShallBeRetried(reply)) {
            q->d_ptr2->numRetries += 1;
            QTimer::singleShot(q->d_ptr2->getRetryDelayInMilliseconds(reply), q,
                               &WebDAVListFilesJob::start);
            return;
        }
        if (reply->error() == QNetworkReply::ProtocolInvalidOperationError && !retryWithDepthZero) {
            // https://gitlab.com/rpdev/opentodolist/-/issues/471
            // Doing a "listing" on a file might cause this error on some servers.
            // Hence, catch it and retry, but this time with a depth of "0":
            retryWithDepthZero = true;
            q->d_ptr2->nextUrl.clear();
            q->start();
            return;
        } else if (reply->error() != QNetworkReply::NoError) {
            q->setError(q->fromNetworkError(*reply), reply->errorString());
            if (!retryWithoutTrailingSlash && !q->url().path().endsWith("/")) {
                // Check if this is a potential error which is due to we append a trailing
                // slash. If so, retry without appending:
                switch (q->error()) {
                case JobError::NetworkRequestFailed:
                    retryWithoutTrailingSlash = true;
                    q->d_ptr2->nextUrl.clear();
                    q->setError(JobError::NoError, QString());
                    q->start();
                    return;
                default:
                    break;
                }
            }
            q->finishLater();
        } else if (q->d_ptr2->shouldFollowUnhandledRedirect(reply)) {
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
                    for (const auto& entry : qAsConst(entryList)) {
                        if (entry.name() != ".") {
                            entries << entry;
                        } else {
                            q->setFolder(entry);
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
