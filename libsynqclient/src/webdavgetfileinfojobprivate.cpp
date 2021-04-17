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

#include "webdavgetfileinfojobprivate.h"

#include "abstractwebdavjobprivate.h"

#include <iostream>

namespace SynqClient {

WebDAVGetFileInfoJobPrivate::WebDAVGetFileInfoJobPrivate(WebDAVGetFileInfoJob* q)
    : GetFileInfoJobPrivate(q)
{
}

void WebDAVGetFileInfoJobPrivate::checkParameters()
{
    Q_Q(WebDAVGetFileInfoJob);
    if (!q->networkAccessManager()) {
        q->setError(JobError::MissingParameter, "No QNetworkAccessManager set");
    }
    if (!q->url().isValid()) {
        q->setError(JobError::MissingParameter, "No URL set");
    }
}

void WebDAVGetFileInfoJobPrivate::handleRequestFinished()
{
    Q_Q(WebDAVGetFileInfoJob);
    std::cerr << "=============================" << std::endl;
    std::cerr << "WebDAVGetFileInfoJob finished" << std::endl;
    std::cerr << "=============================" << std::endl;
    auto reply = q->d_ptr2->reply;
    q->d_ptr2->reply = nullptr;
    if (reply) {
        reply->deleteLater();
        for (const auto& headerPair : reply->rawHeaderPairs()) {
            std::cerr << qUtf8Printable(headerPair.first) << ": "
                      << qUtf8Printable(headerPair.second) << std::endl;
        }
        if (reply->error() != QNetworkReply::NoError) {
            std::cerr << "Reply finished with an error " << reply->error() << " "
                      << qUtf8Printable(reply->errorString()) << std::endl;
            q->setError(q->fromNetworkError(*reply), reply->errorString());
            q->finishLater();
        } else if (q->d_ptr2->shouldFollowUnhandledRedirect(reply)) {
            // Encountered redirect not handled by Qt, follow:
            std::cerr << "Following redirect" << std::endl;
            q->start();
            return;
        } else {
            auto code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            std::cerr << "Finished with code " << code.toInt() << std::endl;
            auto data = reply->readAll();
            std::cerr << qUtf8Printable(data) << std::endl;
            if (code.toInt() == q->d_ptr2->WebDAVMultiStatus) {
                bool ok;

                auto entryList = q->d_ptr2->parseEntryList(reply->url(), data, ok);
                if (!ok) {
                    q->setError(JobError::InvalidResponse,
                                "PROPFIND response from server is not valid");
                } else if (entryList.length() == 1) {
                    auto entry = entryList.at(0);
                    if (entry.name() == ".") {
                        fileInfo = entry;
                    }
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
