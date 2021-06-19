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

#include "../inc/SynqClient/dropboxdownloadfilejob.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

#include "abstractdropboxjobprivate.h"
#include "dropboxdownloadfilejobprivate.h"

namespace SynqClient {

/**
 * @class DropboxDownloadFileJob
 * @brief Implementation of the DownloadFileJob for Dropbox.
 */

/**
 * @brief Constructor.
 */
DropboxDownloadFileJob::DropboxDownloadFileJob(QObject* parent)
    : DownloadFileJob(new DropboxDownloadFileJobPrivate(this), parent), AbstractDropboxJob()
{
}

/**
 * @brief Destructor.
 */
DropboxDownloadFileJob::~DropboxDownloadFileJob() {}

/**
 * @brief Implementation of AbstractJob::start().
 */
void DropboxDownloadFileJob::start()
{
    Q_D(DropboxDownloadFileJob);
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

    if (d->downloadDevice) {
        if (d->downloadDevice != d->output) {
            delete d->downloadDevice;
            d->downloadDevice = nullptr;
        }
    }
    d->downloadDevice = getDownloadDevice();

    if (!d->downloadDevice) {
        finishLater();
        return;
    }

    QVariantMap data { { "path", AbstractDropboxJobPrivate::fixPath(d->remoteFilename) } };

    d->downloadDevice->seek(0);
    auto reply = d_ptr2->postData("/files/download", data, nullptr);

    if (reply) {
        connect(reply, &QNetworkReply::readyRead, this, [=]() {
            if (d->downloadDevice) {
                d->downloadDevice->write(reply->readAll());
            }
        });
        connect(reply, &QNetworkReply::finished, this, [=]() {
            reply->deleteLater();
            if (d_ptr2->checkIfRequestShallBeRetried(reply)) {
                d_ptr2->numRetries += 1;
                QTimer::singleShot(d_ptr2->getRetryDelayInMilliseconds(reply), this,
                                   &DropboxDownloadFileJob::start);
                return;
            }
            if (reply->error() == QNetworkReply::NoError) {
                QJsonParseError error;
                auto doc = QJsonDocument::fromJson(reply->rawHeader("Dropbox-API-Result"), &error);
                if (error.error == QJsonParseError::NoError) {
                    setFileInfo(d_ptr2->fileInfoFromJson(doc.object(), QString(), "file"));
                    if (d->downloadDevice) {
                        d->downloadDevice->write(reply->readAll());
                    }
                } else {
                    setError(JobError::InvalidResponse,
                             tr("Failed to parse JSON response: %s").arg(error.errorString()));
                }
            } else {
                // Unrecognized error - "fail generically"
                setError(JobError::NetworkRequestFailed,
                         reply->errorString() + " " + reply->readAll());
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
void DropboxDownloadFileJob::stop()
{
    if (state() == JobState::Running) {
        auto reply = d_ptr2->reply;
        if (reply) {
            reply->abort();
            delete reply;
        }
        setError(JobError::Stopped, "The job has been stopped");
        finishLater();
    }
}

/**
 * @brief Destructor.
 */
DropboxDownloadFileJob::DropboxDownloadFileJob(DropboxDownloadFileJobPrivate* d, QObject* parent)
    : DownloadFileJob(d, parent), AbstractDropboxJob()
{
}

} // namespace SynqClient
