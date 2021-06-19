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

#include "SynqClient/uploadfilejob.h"

#include <QBuffer>
#include <QFile>
#include <QIODevice>

#include "uploadfilejobprivate.h"

namespace SynqClient {

/**
 * @class UploadFileJob
 * @brief Upload a file to a remote server.
 *
 * This is an abstract base class for jobs that upload a file to a remote server. The job is
 * configured with a path to a remote file. Additionally, some local form of data must be provided.
 * For this, three methods can be used:
 *
 * - setLocalFilename()
 * - setData()
 * - setInput()
 *
 * Only the one called last will be considered, e.g. if you first use setLocalFilename() to
 * configure a path to a local file and afterwards setData() to set the raw data to upload, then the
 * latter will *win*.
 *
 * After a successful upload, information about the remote file can be retrieved using fileInfo().
 * This is useful to access e.g. sync attributes of a remote file.
 *
 * # Error Handling
 *
 * Besides the usual error codes, upload jobs use the following errors to warn about non-fatal
 * errors:
 *
 * - JobError::SyncAttributeMismatch: The remote file was updated and the specified syncAttribute()
 * no longer matches. In other words, there was a *lost update*.
 */

/**
 * @brief Constructor.
 */
UploadFileJob::UploadFileJob(QObject* parent) : AbstractJob(new UploadFileJobPrivate(this), parent)
{
}

/**
 * @brief Destructor.
 */
UploadFileJob::~UploadFileJob() {}

/**
 * @brief The path to a local file to upload.
 */
QString UploadFileJob::localFilename() const
{
    Q_D(const UploadFileJob);
    return d->localFilename;
}

/**
 * @brief Set the path to a local file to be uploaded.
 *
 * @note If previously setInput() or setData() have been used, their respecive configured
 * values are discarded.
 */
void UploadFileJob::setLocalFilename(const QString& localFilename)
{
    Q_D(UploadFileJob);
    d->localFilename = localFilename;
    d->input.reset();
    d->data.clear();
    d->sourceType = UploadFileJobPrivate::UploadSource::Path;
}

/**
 * @brief The input device to read data from for uploading.
 */
QIODevice* UploadFileJob::input() const
{
    Q_D(const UploadFileJob);
    return d->input.data();
}

/**
 * @brief Set the input device to read data from.
 *
 * This sets the @p input device from which the data to be uplaoded is taken from.
 * Note that this must be a sequential device with a known size.
 *
 * @note The job takes ownership of the device. It will be destroyed once the job is destroyed.
 *
 * @note If you previously used setLocalFilename() or setData(), their value will be discarded.
 */
void UploadFileJob::setInput(QIODevice* input)
{
    Q_D(UploadFileJob);
    d->localFilename.clear();
    d->input.reset(input);
    d->data.clear();
    if (input) {
        d->sourceType = UploadFileJobPrivate::UploadSource::IODevice;
    } else {
        d->sourceType = UploadFileJobPrivate::UploadSource::Invalid;
    }
}

/**
 * @brief The data to be uploaded.
 */
QByteArray UploadFileJob::data() const
{
    Q_D(const UploadFileJob);
    return d->data;
}

/**
 * @brief Set the @p data to be uploaded.
 *
 * @note If you previously used setLocalFilename() or setInput(), their data will be discarded.
 */
void UploadFileJob::setData(const QByteArray& data)
{
    Q_D(UploadFileJob);
    d->localFilename.clear();
    d->input.reset();
    d->data = data;
    d->sourceType = UploadFileJobPrivate::UploadSource::Data;
}

/**
 * @brief The path to the remote file to upload to.
 */
QString UploadFileJob::remoteFilename() const
{
    Q_D(const UploadFileJob);
    return d->remoteFilename;
}

/**
 * @brief Set the path to the remote file to upload to.
 */
void UploadFileJob::setRemoteFilename(const QString& remoteFilename)
{
    Q_D(UploadFileJob);
    d->remoteFilename = remoteFilename;
}

/**
 * @brief Meta information about the file just uploaded.
 *
 * This returns meta information about the file which has just been
 * uploaded. This can be used e.g. to learn the new sync property after creating or updating a
 * remote file.
 */
FileInfo UploadFileJob::fileInfo() const
{
    Q_D(const UploadFileJob);
    return d->fileInfo;
}

/**
 * @brief Upload only if the remote and this sync attribute matches.
 *
 * If this property is set to a valid (i.e. non-null) sync-attribute, the
 * upload should only succeed if the remote current sync attribute matches
 * the one set here. This can be used to prevent lost updates.
 *
 * @note The implementation of this is specific to the concrete protocols used.
 * If a protocol does not provide measures to handle conditional
 * uploads, it might be ignored completely.
 *
 * @sa FileInfo
 */
QVariant UploadFileJob::syncAttribute() const
{
    Q_D(const UploadFileJob);
    return d->syncAttribute;
}

/**
 * @brief Set the expected @p syncAttribute of the remote file.
 */
void UploadFileJob::setSyncAttribute(const QVariant& syncAttribute)
{
    Q_D(UploadFileJob);
    d->syncAttribute = syncAttribute;
}

/**
 * @brief Constructor.
 */
UploadFileJob::UploadFileJob(UploadFileJobPrivate* d, QObject* parent) : AbstractJob(d, parent) {}

/**
 * @brief Get the device to use for uploading data.
 *
 * This returns a shared pointer to a QIODevice used for uploading data. The device is constructed
 * from the set input source. If the input is invalid (e.g. not set, points to a non-existing
 * file, etc), a nullptr is returned and an appropriate error is set.
 *
 * @sa error()
 */
QSharedPointer<QIODevice> UploadFileJob::getUploadDevice()
{
    Q_D(UploadFileJob);
    switch (d->sourceType) {
    case UploadFileJobPrivate::UploadSource::Invalid:
        setError(JobError::MissingParameter, "No input set for upload");
        return nullptr;
    case UploadFileJobPrivate::UploadSource::Data: {
        auto result = QSharedPointer<QIODevice>(new QBuffer(&d->data));
        result->open(QIODevice::ReadOnly);
        return result;
    }
    case UploadFileJobPrivate::UploadSource::IODevice:
        if (d->input.isNull()) {
            setError(JobError::InvalidParameter, "Input device set to nullptr");
        }
        d->input->seek(0);
        return d->input;
    case UploadFileJobPrivate::UploadSource::Path:
        QFile* file = new QFile(d->localFilename);
        if (file->open(QIODevice::ReadOnly)) {
            return QSharedPointer<QIODevice>(file);
        } else {
            setError(JobError::InvalidParameter,
                     QString("Failed to open %1 for reading: %2")
                             .arg(d->localFilename, file->errorString()));
            delete file;
            return nullptr;
        }
        break;
    }
    setError(JobError::InvalidParameter, "Unexpected configuration for upload");
    return nullptr;
}

/**
 * @brief Set file meta information.
 *
 * This method shall be used by classes implementing this job to set the file
 * meta information for the file which has been uploaded.
 */
void UploadFileJob::setFileInfo(const FileInfo& fileInfo)
{
    Q_D(UploadFileJob);
    d->fileInfo = fileInfo;
}

} // namespace SynqClient
