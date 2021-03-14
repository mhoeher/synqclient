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

#ifndef SYNQCLIENT_DOWNLOADFILEJOB_H
#define SYNQCLIENT_DOWNLOADFILEJOB_H

#include <QByteArray>
#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QVariantMap>
#include <QtGlobal>

#include "AbstractJob"
#include "FileInfo"
#include "libsynqclient_global.h"

class QIODevice;

namespace SynqClient {

class DownloadFileJobPrivate;

class LIBSYNQCLIENT_EXPORT DownloadFileJob : public AbstractJob
{
    Q_OBJECT
public:
    explicit DownloadFileJob(QObject* parent = nullptr);
    ~DownloadFileJob() override;

    QString localFilename() const;
    void setLocalFilename(const QString& localFilename);

    QIODevice* output() const;
    void setOutput(QIODevice* output);

    QByteArray data() const;

    QString remoteFilename() const;
    void setRemoteFilename(const QString& remoteFilename);

    FileInfo fileInfo() const;

protected:
    explicit DownloadFileJob(DownloadFileJobPrivate* d, QObject* parent = nullptr);

    Q_DECLARE_PRIVATE(DownloadFileJob);

    QIODevice* getDownloadDevice();

    void setFileInfo(const FileInfo& fileInfo);
};

} // namespace SynqClient

#endif // SYNQCLIENT_DOWNLOADFILEJOB_H
