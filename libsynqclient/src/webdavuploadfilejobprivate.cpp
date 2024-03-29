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

#include "webdavuploadfilejobprivate.h"

#include <QTimer>
#include <QLoggingCategory>

#include "abstractwebdavjobprivate.h"

namespace SynqClient {

static Q_LOGGING_CATEGORY(log, "SynqClient.WebDAVUploadFileJob", QtDebugMsg);

WebDAVUploadFileJobPrivate::WebDAVUploadFileJobPrivate(WebDAVUploadFileJob* q)
    : UploadFileJobPrivate(q), uploadDevice(nullptr)
{
}

void WebDAVUploadFileJobPrivate::checkParameters()
{
    Q_Q(WebDAVUploadFileJob);
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

void WebDAVUploadFileJobPrivate::handleRequestFinished()
{
    Q_Q(WebDAVUploadFileJob);
    auto reply = q->d_ptr2->reply;
    q->d_ptr2->reply = nullptr;
    if (reply) {
        reply->deleteLater();
        if (q->d_ptr2->checkIfRequestShallBeRetried(reply)) {
            q->d_ptr2->numRetries += 1;
            QTimer::singleShot(q->d_ptr2->getRetryDelayInMilliseconds(reply), q,
                               &WebDAVUploadFileJob::start);
            return;
        }
        auto code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (reply->error() != QNetworkReply::NoError) {
            if (code == AbstractWebDAVJobPrivate::HTTPPreconditionFailed) {
                q->setError(JobError::SyncAttributeMismatch, "The file on the server was updated");
            } else {
                q->setError(q->fromNetworkError(*reply), reply->errorString());
            }
            q->finishLater();
        } else if (q->d_ptr2->shouldFollowUnhandledRedirect(reply)) {
            // Encountered redirect not handled by Qt, follow:
            q->start();
            return;
        } else {
            FileInfo fileInfo;
            fileInfo.setIsFile();
            QVariant etag = reply->header(QNetworkRequest::ETagHeader);
            if (etag.isValid()) {
                // TODO: Some servers (e.g. Apache's mod_dav) don't include an etag on
                // upload. On the other side, we cannot just do another PROPFIND to get
                // the "current" etag, as another client might meanwhile also have written and then
                // we would take over another file version's etag. The best we can is probably just
                // keep the etag empty and live with it...
                fileInfo.setSyncAttribute(etag.toString());
            } else {
                qCDebug(log) << "Did not receive an eTag on upload";
            }
            q->setFileInfo(fileInfo);
            if (code == q->d_ptr2->HTTPOkay || code == q->d_ptr2->HTTPCreated
                || code == q->d_ptr2->HTTPNoContent) {
                // Pass!
            } else {
                q->setError(JobError::InvalidResponse,
                            QString("Received invalid response from server: %1").arg(code));
            }
            q->finishLater();
        }
    }
}

} // namespace SynqClient
