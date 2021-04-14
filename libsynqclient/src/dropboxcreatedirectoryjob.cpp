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

#include "../inc/SynqClient/dropboxcreatedirectoryjob.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "dropboxcreatedirectoryjobprivate.h"
#include "abstractdropboxjobprivate.h"

namespace SynqClient {

/**
 * @class DropboxCreateDirectoryJob
 * @brief Implementation of the CreateDirectoryJob for Dropbox.
 *
 * The Dropbox implementation of this job class differs in the following points from the defined
 * interface:
 *
 * - *Recursive Folder Creation:* Multiple folders can be created recursively in one command.
 */

/**
 * @brief Constructor.
 */
DropboxCreateDirectoryJob::DropboxCreateDirectoryJob(QObject* parent)
    : CreateDirectoryJob(new DropboxCreateDirectoryJobPrivate(this), parent), AbstractDropboxJob()
{
}

/**
 * @brief Destructor.
 */
DropboxCreateDirectoryJob::~DropboxCreateDirectoryJob() {}

/**
 * @brief Implementation of AbstractJob::start().
 */
void DropboxCreateDirectoryJob::start()
{
    Q_D(DropboxCreateDirectoryJob);
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

    if (d->path.isEmpty()) {
        setError(JobError::MissingParameter, tr("No path specified"));
        finishLater();
        return;
    }

    QVariantMap data { { "path", d->path }, { "autorename", false } };

    auto reply = d_ptr2->post("/files/create_folder_v2", data);

    if (reply) {
        connect(reply, &QNetworkReply::finished, this, [=]() {
            reply->deleteLater();
            if (reply->error() == QNetworkReply::NoError) {
                // Everything fine!
            } else {
                // Try to check if the server replied with an known error description:
                QJsonParseError error;
                auto doc = QJsonDocument::fromJson(reply->readAll(), &error);
                if (error.error == QJsonParseError::NoError) {
                    // {"error_summary": "path/conflict/folder/...", "error": {".tag": "path",
                    // "path": {".tag": "conflict", "conflict": {".tag": "folder"}}}}
                    if (doc.object()
                                .value("error")
                                .toObject()
                                .value("path")
                                .toObject()
                                .value("conflict")
                                .toObject()
                                .value(".tag")
                                .toString()
                        == "folder") {
                        setError(JobError::FolderExists,
                                 tr("The remove folder %1 already exists").arg(d->path));
                    }
                }
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
void DropboxCreateDirectoryJob::stop()
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
DropboxCreateDirectoryJob::DropboxCreateDirectoryJob(DropboxCreateDirectoryJobPrivate* d,
                                                     QObject* parent)
    : CreateDirectoryJob(d, parent), AbstractDropboxJob()
{
}

} // namespace SynqClient
