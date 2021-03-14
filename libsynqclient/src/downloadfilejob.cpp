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

#include "SynqClient/downloadfilejob.h"

#include <QBuffer>
#include <QFile>
#include <QIODevice>

#include "downloadfilejobprivate.h"

namespace SynqClient {

/**
 * @class DownloadFileJob
 * @brief Download files from a remote server.
 *
 * This is an abstract base class for jobs downloading files from a remote server. The job is
 * configured with a path to the remote file. Additionally, one can use either setLocalFilename() or
 * setOutput() to configure an output file or device for saving data into. If no output is
 * configured, the downloaded data can be accessed using the data() method.
 *
 * In addition to downloading the file data, meta information about the remote file can be accessed
 * using fileInfo().
 */

/**
 * @brief Constructor.
 */
DownloadFileJob::DownloadFileJob(QObject* parent)
    : AbstractJob(new DownloadFileJobPrivate(this), parent)
{
}

/**
 * @brief Destructor.
 */
DownloadFileJob::~DownloadFileJob() {}

/**
 * @brief The path to the local file to store downloaded data in.
 */
QString DownloadFileJob::localFilename() const
{
    Q_D(const DownloadFileJob);
    return d->localFilename;
}

/**
 * @brief Set the path to the local file to store downloaded data in.
 *
 * @note If you previously used setOutput(), the configured device will be discarded.
 */
void DownloadFileJob::setLocalFilename(const QString& localFilename)
{
    Q_D(DownloadFileJob);
    d->targetType = DownloadFileJobPrivate::DownloadTarget::Path;
    d->localFilename = localFilename;
}

/**
 * @brief The output device to write data to.
 */
QIODevice* DownloadFileJob::output() const
{
    Q_D(const DownloadFileJob);
    return d->output;
}

/**
 * @brief Set the device to write received data into.
 *
 * This sets the output device into which data is written. Note that this class does not take
 * ownershop of the device - it is up to the caller to delete it later (or parent it to the job
 * so it is deleted together with it).
 *
 * If output is a nullptr, the downloaded data will be buffered and can be accessed using data()
 * after the job succeeded.
 */
void DownloadFileJob::setOutput(QIODevice* output)
{
    Q_D(DownloadFileJob);
    d->output = output;
    if (output) {
        d->targetType = DownloadFileJobPrivate::DownloadTarget::IODevice;
    } else {
        d->targetType = DownloadFileJobPrivate::DownloadTarget::Data;
    }
}

/**
 * @brief Get the data of the file which has been downloaded.
 *
 * This returns the raw data of the downloaded file.
 */
QByteArray DownloadFileJob::data() const
{
    Q_D(const DownloadFileJob);
    return d->data;
}

/**
 * @brief The path to the remote file to be downloaded.
 */
QString DownloadFileJob::remoteFilename() const
{
    Q_D(const DownloadFileJob);
    return d->remoteFilename;
}

/**
 * @brief Set the path to the remote file to be downloaded.
 */
void DownloadFileJob::setRemoteFilename(const QString& remoteFilename)
{
    Q_D(DownloadFileJob);
    d->remoteFilename = remoteFilename;
}

/**
 * @brief Meta information about the downloaded file.
 *
 * After a successful download, this provides meta information about the downloaded file. In
 * particular, this provides the remote sync attribute of the file.
 */
FileInfo DownloadFileJob::fileInfo() const
{
    Q_D(const DownloadFileJob);
    return d->fileInfo;
}

/**
 * @brief Constructor.
 */
DownloadFileJob::DownloadFileJob(DownloadFileJobPrivate* d, QObject* parent)
    : AbstractJob(d, parent)
{
}

/**
 * @brief Get a QIODevice to write received data to.
 *
 * This method returns a QIODevice which can be used to write the received data to.
 * If an output device has been configured, it is returned. Otherwise, depending on if a
 * local file name is set this either returns a QFile instance which can be used to write
 * to the specified file or a QBuffer instance which can be used to write the received data into
 * a buffer in memory. In the latter cases, the returned device is parented to this object. If
 * a valid output device is set, it is returned as-is.
 *
 * If creating a suitable output device fails, a nullptr is returned and a job error is set.
 */
QIODevice* DownloadFileJob::getDownloadDevice()
{
    Q_D(DownloadFileJob);
    switch (d->targetType) {
    case DownloadFileJobPrivate::DownloadTarget::Data: {
        auto buffer = new QBuffer(&d->data);
        buffer->setParent(this);
        if (!buffer->open(QIODevice::WriteOnly)) {
            setError(JobError::InvalidParameter, "Failed to create local data buffer");
            delete buffer;
            buffer = nullptr;
        }
        return buffer;
    }

    case DownloadFileJobPrivate::DownloadTarget::IODevice:
        if (!d->output) {
            setError(JobError::MissingParameter, "No output device set");
        }
        return d->output;

    case DownloadFileJobPrivate::DownloadTarget::Path: {
        auto file = new QFile(d->localFilename);
        file->setParent(this);
        if (!file->open(QIODevice::WriteOnly)) {
            setError(JobError::InvalidParameter,
                     QString("Failed to open %1 for writing: %2")
                             .arg(d->localFilename, file->errorString()));
            delete file;
            file = nullptr;
        }
        return file;
    }
    }

    setError(JobError::InvalidParameter, "Unexpected configuration for download");
    return nullptr;
}

/**
 * @brief Set meta information about the remote file.
 *
 * Concrete sub-classes shall use this method to set file meta information for the remote file after
 * a successful download.
 */
void DownloadFileJob::setFileInfo(const FileInfo& fileInfo)
{
    Q_D(DownloadFileJob);
    d->fileInfo = fileInfo;
}

} // namespace SynqClient
