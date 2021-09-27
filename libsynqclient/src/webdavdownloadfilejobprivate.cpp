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

#include "webdavdownloadfilejobprivate.h"

#include <QLoggingCategory>
#include <QRegularExpression>
#include <QTimer>

#include "abstractwebdavjobprivate.h"

namespace SynqClient {

static Q_LOGGING_CATEGORY(log, "SynqClient.WebDAVDownloadFileJob", QtDebugMsg);

WebDAVDownloadFileJobPrivate::WebDAVDownloadFileJobPrivate(WebDAVDownloadFileJob* q)
    : DownloadFileJobPrivate(q), downloadDevice(nullptr)
{
}

void WebDAVDownloadFileJobPrivate::checkParameters()
{
    Q_Q(WebDAVDownloadFileJob);
    if (!q->networkAccessManager()) {
        q->setError(JobError::MissingParameter, "No QNetworkAccessManager set");
    }
    if (!q->url().isValid()) {
        q->setError(JobError::MissingParameter, "No URL set");
    }
    if (q->remoteFilename().isEmpty()) {
        q->setError(JobError::MissingParameter, "No remote file name set");
    }
}

void WebDAVDownloadFileJobPrivate::handleRequestFinished()
{
    Q_Q(WebDAVDownloadFileJob);
    auto reply = q->d_ptr2->reply;
    q->d_ptr2->reply = nullptr;
    if (reply) {
        reply->deleteLater();
        if (q->d_ptr2->checkIfRequestShallBeRetried(reply)) {
            q->d_ptr2->numRetries += 1;
            QTimer::singleShot(q->d_ptr2->getRetryDelayInMilliseconds(reply), q,
                               &WebDAVDownloadFileJob::start);
            return;
        }
        if (reply->error() != QNetworkReply::NoError) {
            q->setError(q->fromNetworkError(*reply), reply->errorString());
            q->finishLater();
        } else if (q->d_ptr2->shouldFollowUnhandledRedirect(reply)) {
            // Encountered redirect not handled by Qt, follow:
            q->start();
            return;
        } else {
            auto code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (code == q->d_ptr2->HTTPOkay) {
                if (downloadDevice) {
                    downloadDevice->write(reply->readAll());
                }
                auto etag = reply->header(QNetworkRequest::ETagHeader);
                FileInfo fileInfo;
                fileInfo.setIsFile();
                if (etag.isValid()) {
                    auto etagString = etag.toString();

                    // Convert "weak" etags to normal ones:
                    if (etagString.startsWith("W/")) {
                        auto newEtag = etagString.mid(2);
                        qCDebug(log)
                                << "Converting weak etag from" << etagString << "to" << newEtag;
                        etagString = newEtag;
                    }

                    // Workaround for https://gitlab.com/rpdev/opentodolist/-/issues/471:
                    // If we detect a known pattern, try to extract the relevant part of the ETag
                    // (which matches the ones we get via PROPFIND requests):
                    if (q->d_ptr2->workarounds.testFlag(
                                WebDAVWorkaround::DerivePROPFINDETagsFromGETETagsForApache)) {
                        // Check if the etag follows a known (error) pattern.
                        auto re = QRegularExpression(R"(^"[0-9a-f]+-([0-9a-f]+-[0-9a-f]+")$)");
                        auto match = re.match(etagString);
                        if (match.hasMatch()) {
                            auto newEtagString = "\"" + match.captured(1);
                            qCDebug(log) << "Detected possible broken ETag" << etagString
                                         << "- converting to" << newEtagString;
                            etagString = newEtagString;
                        }
                    }

                    fileInfo.setSyncAttribute(etagString);
                }
                q->setFileInfo(fileInfo);
            } else {
                q->setError(JobError::InvalidResponse,
                            QString("Received invalid response from server: %1").arg(code));
            }
            q->finishLater();
        }
    }
}

} // namespace SynqClient
