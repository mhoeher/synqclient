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

#ifndef SYNQCLIENT_NEXTCLOUDLOGINFLOW_H
#define SYNQCLIENT_NEXTCLOUDLOGINFLOW_H

#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>

#include "libsynqclient_global.h"

class QNetworkAccessManager;

namespace SynqClient {

class NextCloudLoginFlowPrivate;

class LIBSYNQCLIENT_EXPORT NextCloudLoginFlow : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool flowRunning READ flowRunning NOTIFY flowRunningChanged);

public:
    explicit NextCloudLoginFlow(QObject* parent = nullptr);
    ~NextCloudLoginFlow() override;

    bool flowRunning() const;

    QNetworkAccessManager* networkAccessManager() const;
    void setNetworkAccessManager(QNetworkAccessManager* networkAccessManager);

    QString userAgent() const;
    void setUserAgent(const QString& userAgent);

    Q_INVOKABLE bool startLoginFlow(const QUrl& url);

signals:

    /**
     * @brief The login flow has finished.
     *
     * This signal is emitted once to indicate that the login flow has finished. This signal is
     * emitted regardless on whether the flow succeeded or not.
     */
    void finished();

    /**
     * @brief The flowRunning property changed.
     */
    void flowRunningChanged();

    /**
     * @brief The login @p url has been received.
     *
     * This signal is emitted during the flow. It carries the login URL that the user has to
     * visit in a browser. On that page, the user will need to log in and grant access to the app
     * to continue the flow.
     *
     * Client code should listen to this signal and open a browser so the flow can continue:
     *
     * @code
     * connect(flow, &SynqClient::NextCloudLoginFlow::receivedLoginUrl, [=](const QUrl &url) {
     *     QDesktopServices::openUrl(url);
     * });
     * @endcode
     */
    void receivedLoginUrl(const QUrl& url);

    /**
     * @brief The login credentials have been received.
     *
     * This signal is emitted once to indicate that the user granted access and we received
     * the needed @p username, app-specific @p password as well as the @p server URL to be used
     * to talk to the NextCloud server.
     */
    void receivedLogin(const QString& username, const QString& password, const QUrl& server);

protected:
    explicit NextCloudLoginFlow(NextCloudLoginFlowPrivate* d, QObject* parent = nullptr);

    QScopedPointer<NextCloudLoginFlowPrivate> d_ptr;
    Q_DECLARE_PRIVATE(NextCloudLoginFlow);
};

} // namespace SynqClient

#endif // SYNQCLIENT_NEXTCLOUDLOGINFLOW_H
