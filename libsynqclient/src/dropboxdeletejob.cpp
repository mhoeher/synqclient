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

#include "../inc/SynqClient/dropboxdeletejob.h"

#include "abstractdropboxjobprivate.h"
#include "dropboxdeletejobprivate.h"

namespace SynqClient {

/**
 * @class DropboxDeleteJob
 * @brief Implementation of the DeleteJob for Dropbox.
 */

/**
 * @brief Constructor.
 */
DropboxDeleteJob::DropboxDeleteJob(QObject* parent)
    : DeleteJob(new DropboxDeleteJobPrivate(this), parent), AbstractDropboxJob()
{
}

/**
 * @brief Destructor.
 */
DropboxDeleteJob::~DropboxDeleteJob() {}

/**
 * @brief Implementation of AbstractJob::start().
 */
void DropboxDeleteJob::start()
{
    Q_D(DropboxDeleteJob);
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

    if (syncAttribute().isValid()) {
        data["parent_rev"] = syncAttribute();
    }

    auto reply = d_ptr2->post("/files/delete_v2", data);

    if (reply) {
        connect(reply, &QNetworkReply::finished, this, [=]() {
            reply->deleteLater();
            if (reply->error() == QNetworkReply::NoError) {
                // Done!
            } else {
                // Check if this is a "known" error
                auto notAnError = false;
                auto errorData = reply->readAll();
                d_ptr2->tryHandleKnownError(
                        errorData,
                        { { { { "error", "path", ".tag" }, "not_found" },
                            [&](const QJsonDocument&) {
                                // Resource no longer present - all good!
                                notAnError = true;
                            } },
                          // {"error_summary": "path_write/conflict/file/..", "error": {".tag":
                          // "path_write", "path_write": {".tag": "conflict", "conflict": {".tag":
                          // "file"}}}}
                          { { { "error", "path_write", "conflict", ".tag" }, "file" },
                            [=](const QJsonDocument&) {
                                setError(JobError::SyncAttributeMismatch,
                                         tr("The file on the server was updated"));
                            } } });

                if (this->error() == JobError::NoError && !notAnError) {
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
void DropboxDeleteJob::stop()
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
DropboxDeleteJob::DropboxDeleteJob(DropboxDeleteJobPrivate* d, QObject* parent)
    : DeleteJob(d, parent), AbstractDropboxJob()
{
}

} // namespace SynqClient
