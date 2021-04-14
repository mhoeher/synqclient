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

#ifndef SYNQCLIENT_ABSTRACTDROPBOXJOBPRIVATE_H
#define SYNQCLIENT_ABSTRACTDROPBOXJOBPRIVATE_H

#include <tuple>

#include <QCoreApplication>
#include <QNetworkReply>
#include <QPointer>

#include "SynqClient/FileInfo"
#include "SynqClient/SynqClient"
#include "SynqClient/abstractdropboxjob.h"

class QJsonObject;

namespace SynqClient {

class AbstractDropboxJobPrivate
{

    Q_DECLARE_TR_FUNCTIONS(AbstractDropboxJobPrivate)

public:
    static const QString APIv2;

    explicit AbstractDropboxJobPrivate(AbstractDropboxJob* q);
    virtual ~AbstractDropboxJobPrivate();

    AbstractDropboxJob* q_ptr;
    Q_DECLARE_PUBLIC(AbstractDropboxJob);

    QPointer<QNetworkAccessManager> networkAccessManager;
    QString userAgent;
    QString token;

    QPointer<QNetworkReply> reply;

    std::tuple<JobError, QString> checkDefaultParameters();

    static FileInfo fileInfoFromJson(const QJsonObject& obj, const QString& basePath = QString());

    QNetworkReply* post(const QString& endpoint, const QVariant& data);
};

} // namespace SynqClient

#endif // SYNQCLIENT_ABSTRACTDROPBOXJOBPRIVATE_H
