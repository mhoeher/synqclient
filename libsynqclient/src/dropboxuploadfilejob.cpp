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

#include "../inc/SynqClient/dropboxuploadfilejob.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "abstractdropboxjobprivate.h"
#include "dropboxuploadfilejobprivate.h"

namespace SynqClient {

/**
 * @class DropboxUploadFileJob
 * @brief Implementation of the UploadFileJob for Dropbox.
 *
 * This class implements the abstract UploadFileJob class for Dropbox. It behaves specially
 * in the following aspects:
 *
 * - Currently, only files up to 150MB can be uploaded. This is because the job currently only
 *   uses the `/files/upload` endpoint. See
 *   https://www.dropbox.com/developers/documentation/http/documentation#files-upload for some more
 *   details.
 */

/**
 * @brief Constructor.
 */
DropboxUploadFileJob::DropboxUploadFileJob(QObject* parent)
    : UploadFileJob(new DropboxUploadFileJobPrivate(this), parent), AbstractDropboxJob()
{
}

/**
 * @brief Destructor.
 */
DropboxUploadFileJob::~DropboxUploadFileJob() {}

/**
 * @brief Implementation of AbstractJob::start().
 */
void DropboxUploadFileJob::start()
{
    Q_D(DropboxUploadFileJob);
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

    if (!d->uploadDevice) {
        d->uploadDevice = getUploadDevice();
    }

    QVariantMap data { { "path", AbstractDropboxJobPrivate::fixPath(d->remoteFilename) },
                       { "mode", "overwrite" },
                       { "autorename", false },
                       { "mute", true } };
    auto syncAttr = syncAttribute();
    if (!syncAttr.isNull()) {
        data["mode"] = QVariantMap { { ".tag", "update" }, { "update", syncAttr } };
    }

    auto reply = d_ptr2->postData("/files/upload", data, d->uploadDevice.data());

    if (reply) {
        connect(reply, &QNetworkReply::finished, this, [=]() {
            reply->deleteLater();
            if (reply->error() == QNetworkReply::NoError) {
                QJsonParseError error;
                auto doc = QJsonDocument::fromJson(reply->readAll(), &error);
                if (error.error == QJsonParseError::NoError) {
                    setFileInfo(d_ptr2->fileInfoFromJson(doc.object(), QString(), "file"));
                } else {
                    setError(JobError::InvalidResponse,
                             tr("Failed to parse JSON response: %s").arg(error.errorString()));
                }
            } else {
                // {\"error_summary\": \"path/conflict/file/.\", \"error\": {\".tag\": \"path\",
                // \"reason\": {\".tag\": \"conflict\", \"conflict\": {\".tag\": \"file\"}},
                // \"upload_session_id\": \"XXX\"}}"

                // Check if this is a "known" error
                d_ptr2->tryHandleKnownError(
                        reply->readAll(),
                        { { { { "error", "reason", "conflict", ".tag" }, "file" },
                            [=](const QJsonDocument&) {
                                setError(JobError::SyncAttributeMismatch,
                                         tr("The file on the server was updated"));
                            } } });

                if (this->error() == JobError::NoError) {
                    // Unrecognized error - "fail generically"
                    setError(JobError::NetworkRequestFailed,
                             reply->errorString() + " " + reply->readAll());
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
void DropboxUploadFileJob::stop()
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
DropboxUploadFileJob::DropboxUploadFileJob(DropboxUploadFileJobPrivate* d, QObject* parent)
    : UploadFileJob(d, parent), AbstractDropboxJob()
{
}

} // namespace SynqClient
