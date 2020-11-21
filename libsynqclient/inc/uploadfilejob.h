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

#ifndef SYNQCLIENT_UPLOADFILEJOB_H
#define SYNQCLIENT_UPLOADFILEJOB_H

#include <QByteArray>
#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QtGlobal>

#include "AbstractJob"
#include "libsynqclient_global.h"

class QIODevice;

namespace SynqClient {

class UploadFileJobPrivate;

class LIBSYNQCLIENT_EXPORT UploadFileJob : public AbstractJob
{
    Q_OBJECT
public:
    explicit UploadFileJob(QObject* parent = nullptr);
    ~UploadFileJob() override;

    QString localFilename() const;
    void setLocalFilename(const QString& localFilename);

    QIODevice* input() const;
    void setInput(QIODevice* input);

    QByteArray data() const;
    void setData(const QByteArray& data);

    QString remoteFilename() const;
    void setRemoteFilename(const QString& remoteFilename);

    QVariantMap fileInfo() const;

    QVariant syncAttribute() const;
    void setSyncAttribute(const QVariant& syncAttribute);

protected:
    explicit UploadFileJob(UploadFileJobPrivate* d, QObject* parent = nullptr);

    Q_DECLARE_PRIVATE(UploadFileJob);

    QSharedPointer<QIODevice> getUploadDevice();

    void setFileInfo(const QVariantMap& fileInfo);
};

} // namespace SynqClient

#endif // SYNQCLIENT_UPLOADFILEJOB_H
