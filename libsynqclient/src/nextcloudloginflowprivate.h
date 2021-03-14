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

#ifndef SYNQCLIENT_NEXTCLOUDLOGINFLOWPRIVATE_H
#define SYNQCLIENT_NEXTCLOUDLOGINFLOWPRIVATE_H

#include <QPointer>

#include "SynqClient/nextcloudloginflow.h"

class QJsonDocument;
class QNetworkAccessManager;
class QTimer;

namespace SynqClient {

class NextCloudLoginFlowPrivate : public QObject
{
    Q_OBJECT
public:
    explicit NextCloudLoginFlowPrivate(NextCloudLoginFlow* q);

    NextCloudLoginFlow* q_ptr;
    Q_DECLARE_PUBLIC(NextCloudLoginFlow);

    QPointer<QNetworkAccessManager> networkAccessManager;
    QTimer* pollTimer;
    QString token;
    QString endpoint;
    QString userAgent;
    bool flowRunning;

    static bool isValidFlowKickoffResponse(const QJsonDocument& doc, QString& error);
    static bool isValidPollResultResponse(const QJsonDocument& doc, QString& error);

public slots:

    void onFlowKickoffFinished();
    void onPollFinished();
    void poll();
    void parsePollResult(const QByteArray& data);

    void finish();
};

} // namespace SynqClient

#endif // SYNQCLIENT_NEXTCLOUDLOGINFLOWPRIVATE_H
