/*
 * Copyright 2020 Martin Hoeher <martin@rpdev.net>
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

#include "../inc/uploadfilejob.h"

#include <QBuffer>
#include <QFile>
#include <QIODevice>

#include "uploadfilejobprivate.h"

namespace SynqClient {

UploadFileJob::UploadFileJob(QObject* parent) : AbstractJob(new UploadFileJobPrivate(this), parent)
{
}

UploadFileJob::~UploadFileJob() {}

QString UploadFileJob::localFilename() const
{
    Q_D(const UploadFileJob);
    return d->localFilename;
}

void UploadFileJob::setLocalFilename(const QString& localFilename)
{
    Q_D(UploadFileJob);
    d->localFilename = localFilename;
    d->input.reset();
    d->data.clear();
    d->sourceType = UploadFileJobPrivate::UploadSource::Path;
}

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
 * The job takes ownership of the device. It will be destroyed once the job is destroyed.
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

QByteArray UploadFileJob::data() const
{
    Q_D(const UploadFileJob);
    return d->data;
}

void UploadFileJob::setData(const QByteArray& data)
{
    Q_D(UploadFileJob);
    d->localFilename.clear();
    d->input.reset();
    d->data = data;
    d->sourceType = UploadFileJobPrivate::UploadSource::Data;
}

QString UploadFileJob::remoteFilename() const
{
    Q_D(const UploadFileJob);
    return d->remoteFilename;
}

void UploadFileJob::setRemoteFilename(const QString& remoteFilename)
{
    Q_D(UploadFileJob);
    d->remoteFilename = remoteFilename;
}

UploadFileJob::UploadFileJob(UploadFileJobPrivate* d, QObject* parent) : AbstractJob(d, parent) {}

/**
 * @brief Get the device to use for uploading data.
 *
 * This returns a shared pointer to a QIODevice used for uploading data. The device is constructed
 * from the set input source. If the input is invalid (e.g. not set, points to a non-existing
 * file, etc), a nullpts is returned and an appropriate error is set.
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
        return d->input;
    case UploadFileJobPrivate::UploadSource::Path:
        QFile* file = new QFile(d->localFilename);
        if (file->open(QIODevice::ReadOnly)) {
            return QSharedPointer<QIODevice>(file);
        } else {
            setError(JobError::InvalidParameter,
                     QString("Failed to open %1 for reading: %2")
                             .arg(d->localFilename)
                             .arg(file->errorString()));
            delete file;
            return nullptr;
        }
        break;
    }
    setError(JobError::InvalidParameter, "Unexpected configuration for upload");
    return nullptr;
}

} // namespace SynqClient
