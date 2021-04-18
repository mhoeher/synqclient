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

#include "../inc/SynqClient/dropboxlistfilesjob.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "abstractdropboxjobprivate.h"
#include "dropboxlistfilesjobprivate.h"

namespace SynqClient {

/**
 * @class DropboxListFilesJob
 * @brief Implementation of the ListFilesJob for Dropbox.
 */

/**
 * @brief Constructor.
 */
DropboxListFilesJob::DropboxListFilesJob(QObject* parent)
    : ListFilesJob(new DropboxListFilesJobPrivate(this), parent), AbstractDropboxJob()
{
}

/**
 * @brief Destructor.
 */
DropboxListFilesJob::~DropboxListFilesJob() {}

/**
 * @brief Implementation of AbstractJob::start().
 */
void DropboxListFilesJob::start()
{
    Q_D(DropboxListFilesJob);
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

    {
        FileInfo folderInfo;
        folderInfo.setIsDirectory();
        folderInfo.setName(".");
        setFolder(folderInfo);
    }

    QVariantMap data;
    QString endpoint;
    if (d->cursor.isEmpty()) {
        data = QVariantMap { { "path", d->path } };
        endpoint = "/files/list_folder";
    } else {
        data = QVariantMap { { "cursor", d->cursor } };
        endpoint = "/files/list_folder/continue";
    }

    auto reply = d_ptr2->post("/files/list_folder", data);

    if (reply) {
        connect(reply, &QNetworkReply::finished, this, [=]() {
            reply->deleteLater();
            if (reply->error() == QNetworkReply::NoError) {
                QJsonParseError error;
                auto doc = QJsonDocument::fromJson(reply->readAll(), &error);
                if (error.error == QJsonParseError::NoError) {
                    auto docObject = doc.object();
                    auto previousEntries = entries();
                    auto newEntries = docObject.value("entries").toArray();
                    for (int i = 0; i < newEntries.count(); ++i) {
                        auto entry = newEntries.at(i).toObject();
                        previousEntries << d_ptr2->fileInfoFromJson(entry, d->path);
                    }
                    setEntries(previousEntries);
                    if (docObject.value("has_more").toBool(false)) {
                        d->cursor = docObject.value("cursor").toString();
                        start();
                        return;
                    }
                } else {
                    setError(JobError::InvalidResponse,
                             tr("Failed to parse JSON response: %s").arg(error.errorString()));
                }
            } else {
                // Check if this is a "known" error
                d_ptr2->tryHandleKnownError(reply->readAll(),
                                            { { { { "error", "path", ".tag" }, "not_folder" },
                                                [=](const QJsonDocument&) {
                                                    auto folderInfo = folder();
                                                    folderInfo.setIsFile();
                                                    setFolder(folderInfo);
                                                } } });

                if (this->error() == JobError::NoError && folder().isDirectory()) {
                    // Unrecognized error - "fail generically" (except we detected the remote is a
                    // file).
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
void DropboxListFilesJob::stop()
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
DropboxListFilesJob::DropboxListFilesJob(DropboxListFilesJobPrivate* d, QObject* parent)
    : ListFilesJob(d, parent), AbstractDropboxJob()
{
}

} // namespace SynqClient
