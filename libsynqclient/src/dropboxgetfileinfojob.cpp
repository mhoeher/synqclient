/*
 * Copyright 2021 Martin Hoeher <martin@rpdev.net>
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

#include "../inc/SynqClient/dropboxgetfileinfojob.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "abstractdropboxjobprivate.h"
#include "dropboxgetfileinfojobprivate.h"

namespace SynqClient {

/**
 * @class DropboxGetFileInfoJob
 * @brief Implementation of the GetFileInfoJob for Dropbox.
 *
 * This class implements the abstract GetFileInfoJob class for Dropbox. It behaves specially
 * in the following aspects:
 *
 * - No information for the root directory can be retrieved. Trying so will result in an error. See
 * https://www.dropbox.com/developers/documentation/http/documentation#files-get_metadata.
 */

/**
 * @brief Constructor.
 */
DropboxGetFileInfoJob::DropboxGetFileInfoJob(QObject* parent)
    : GetFileInfoJob(new DropboxGetFileInfoJobPrivate(this), parent), AbstractDropboxJob()
{
}

/**
 * @brief Destructor.
 */
DropboxGetFileInfoJob::~DropboxGetFileInfoJob() {}

/**
 * @brief Implementation of AbstractJob::start().
 */
void DropboxGetFileInfoJob::start()
{
    Q_D(DropboxGetFileInfoJob);
    d->state = JobState::Running;

    {
        auto error = d_ptr2->checkDefaultParameters();
        auto code = std::get<0>(error);
        if (code != JobError::NoError) {
            setError(code, std::get<1>(error));
            finishLater();
            return;
        }
    }

    QVariantMap data { { "path", AbstractDropboxJobPrivate::fixPath(d->path) } };

    auto reply = d_ptr2->post("/files/get_metadata", data);

    if (reply) {
        connect(reply, &QNetworkReply::finished, this, [=]() {
            reply->deleteLater();
            if (reply->error() == QNetworkReply::NoError) {
                QJsonParseError error;
                auto doc = QJsonDocument::fromJson(reply->readAll(), &error);
                if (error.error == QJsonParseError::NoError) {
                    setFileInfo(d_ptr2->fileInfoFromJson(doc.object()));
                    if (fileInfo().isDeleted()) {
                        setError(JobError::ResourceDeleted,
                                 tr("The remote resource %s has been deleted").arg(d->path));
                    }
                } else {
                    setError(JobError::InvalidResponse,
                             tr("Failed to parse JSON response: %s").arg(error.errorString()));
                }
            } else {
                // Check if this is a "known" error
                d_ptr2->tryHandleKnownError(
                        reply->readAll(),
                        { { { { "error", "path", ".tag" }, "not_found" },
                            [=](const QJsonDocument&) {
                                setError(JobError::ResourceNotFound,
                                         tr("The remove path %1 does not exist").arg(d->path));
                            } } });

                if (this->error() == JobError::NoError) {
                    // Unrecognized error - "fail generically"
                    setError(JobError::NetworkRequestFailed, reply->errorString());
                }
            }
            finishLater();
        });
        d_ptr2->reply = reply;
    } else {
        setError(JobError::InvalidResponse, tr("Received null network reply"));
        finishLater();
    }
}

/**
 * @brief Implementation of AbstractJob::stop().
 */
void DropboxGetFileInfoJob::stop()
{
    auto reply = d_ptr2->reply;
    if (reply) {
        reply->abort();
        delete reply;
    }
    setError(JobError::Stopped, "The job has been stopped");
    finishLater();
}

/**
 * @brief Constructor.
 */
DropboxGetFileInfoJob::DropboxGetFileInfoJob(DropboxGetFileInfoJobPrivate* d, QObject* parent)
    : GetFileInfoJob(d, parent), AbstractDropboxJob()
{
}

} // namespace SynqClient
