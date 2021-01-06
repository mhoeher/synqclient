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

#ifndef SYNQCLIENT_DOWNLOADFILEJOBPRIVATE_H
#define SYNQCLIENT_DOWNLOADFILEJOBPRIVATE_H

#include <QPointer>
#include <QVariantMap>

#include "abstractjobprivate.h"
#include "downloadfilejob.h"

namespace SynqClient {

class DownloadFileJobPrivate : public AbstractJobPrivate
{
public:
    enum class DownloadTarget { Path, IODevice, Data };

    explicit DownloadFileJobPrivate(DownloadFileJob* q);
    ~DownloadFileJobPrivate() override;

    Q_DECLARE_PUBLIC(DownloadFileJob);

    QString localFilename;
    QPointer<QIODevice> output;
    QByteArray data;
    QString remoteFilename;
    DownloadTarget targetType;
    FileInfo fileInfo;
};

} // namespace SynqClient

#endif // SYNQCLIENT_DOWNLOADFILEJOBPRIVATE_H
