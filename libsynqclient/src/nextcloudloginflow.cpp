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

#include "SynqClient/nextcloudloginflow.h"

#include <QDir>
#include <QLoggingCategory>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "nextcloudloginflowprivate.h"

namespace SynqClient {

static Q_LOGGING_CATEGORY(log, "SynqClient.NextCloudLoginFlow", QtWarningMsg);

/**
 * @class NextCloudLoginFlow
 * @brief Implement the NextCloud login flow.
 *
 * This class implements the NextCloud login flow. The usage is quite simple:
 *
 * - Create an instance of the class.
 * - Configure it appropriately.
 * - Setup signal/slot connections.
 * - And finally: Start the flow.
 *
 * Here is a minimal example:
 *
 * @code
 * auto flow = new SynqClient::NextCloudLoginFlow(this);
 * flow->setNetworkAccessManager(this->nam);
 * flow->setUserAgent("My Cool App");
 * if (flow->startLoginFlow(QUrl("http://nextcloud.example.com"))) {
 *     connect(flow, &SynqClient::NextCloudLoginFlow::receivedLoginUrl,
 *             [=](const QUrl &url) {
 *                 // Open the URL in the user configured browser:
 *                 QDesktopServices::openUrl(url);
 *             });
 *     connect(flow, &SynqClient::NextCloudLoginFlow::receivedLogin,
 *             [=](const QString &username, const QString &password, const QUrl &server) {
 *                 this->username = username;
 *                 this->password = password;
 *                 this->server = server;
 *             });
 *     connect(flow, &SyncClient::NextCloudLoginFlow::finished,
 *             flow, &QObject::deleteLater);
 * }
 * @endcode
 *
 * @note It is important to connect to the receivedLogin() signal and open the URL - either
 *       in the default browser (e.g. on Desktop systems) or in an embedded HTML view
 *       (on mobiles). The user needs to login and explicitly grant access to the app.
 * @note Make sure you set a proper userAgent(). The user agent string will be shown to the
 *       user in the web page where they are asked to grant access.
 * @note The flow does not time out. This means if the user either does not grant access or
 *       simply never finished the flow, the finished() signal will never be emitted. Hence,
 *       either use an additional timer or - better - provide a user interface where the user
 *       can manually enter the username and password to be used.
 */

/**
 * @brief Constructor.
 *
 * Creates a new NextCloudLoginFlow object which is owned by the @p parent object.
 */
NextCloudLoginFlow::NextCloudLoginFlow(QObject* parent)
    : QObject(parent), d_ptr(new NextCloudLoginFlowPrivate(this))
{
}

/**
 * @brief Destructor.
 */
NextCloudLoginFlow::~NextCloudLoginFlow() {}

/**
 * @brief Indicates if the login flow is currently running.
 *
 * This returns true if this object is currently running a login flow or false otherwise.
 */
bool NextCloudLoginFlow::flowRunning() const
{
    Q_D(const NextCloudLoginFlow);
    return d->flowRunning;
}

/**
 * @brief The network access manager used by the flow.
 */
QNetworkAccessManager* NextCloudLoginFlow::networkAccessManager() const
{
    Q_D(const NextCloudLoginFlow);
    return d->networkAccessManager;
}

/**
 * @brief Set the @p networkAccessManager to be used by the flow.
 *
 * This sets the network access manager that shall be used by the login flow.
 *
 * @note Setting this while a flow is already running is undefined behaviour.
 */
void NextCloudLoginFlow::setNetworkAccessManager(QNetworkAccessManager* networkAccessManager)
{
    Q_D(NextCloudLoginFlow);
    d->networkAccessManager = networkAccessManager;
}

/**
 * @brief The user agent used when communicating with the NextCloud server.
 *
 * This holds the user agent used when communicating with the NextCloud server. The
 * default value is "SynqClient". Set it to something meaningful (ideally the name of
 * your app) because the user will be displayed this name in the web interface.
 */
QString NextCloudLoginFlow::userAgent() const
{
    Q_D(const NextCloudLoginFlow);
    return d->userAgent;
}

/**
 * @brief Set the @p userAgent to be used when communicating with the NextCloud server.
 */
void NextCloudLoginFlow::setUserAgent(const QString& userAgent)
{
    Q_D(NextCloudLoginFlow);
    d->userAgent = userAgent;
}

/**
 * @brief Start a new login flow against the @p url.
 *
 * Calling this will start a new login flow against the NextCloud server listening on the
 * specified URL. This basically sends a request against the specific login flow endpoint
 * of the server and immediately returns.
 *
 * If sending the request was successful (and also all other prerequisites are fulfilled)
 * this method returns true and the flowRunning() property will change to true.
 *
 * In case of an error, false is returned. Error causes might be:
 *
 * - No network manager is set.
 * - The network request could not be created.
 * - This object already runs a login flow.
 */
bool NextCloudLoginFlow::startLoginFlow(const QUrl& url)
{
    Q_D(NextCloudLoginFlow);
    qCDebug(log) << "Started NextCloud login flow v2 against" << url;

    if (d->flowRunning) {
        // Flow already runs - return immediately.
        qCWarning(log) << "A login flow is already running";
        return false;
    }

    if (!d->networkAccessManager) {
        qCWarning(log) << "Cannot start login flow without a network access manager";
        return false;
    }

    QDir remotePath(url.path() + "/index.php/login/v2");
    QUrl loginUrl = url;
    loginUrl.setPath(QDir::cleanPath(remotePath.path()));

    QNetworkRequest request;
    request.setUrl(QUrl(loginUrl));
    request.setHeader(QNetworkRequest::UserAgentHeader, d->userAgent);
    auto reply = d->networkAccessManager->post(request, QByteArray());
    if (reply) {
        connect(reply, &QNetworkReply::finished, d,
                &NextCloudLoginFlowPrivate::onFlowKickoffFinished);
        emit flowRunningChanged();
        d->flowRunning = true;
        return true;
    } else {
        // Fatal, should probably not happen:
        qCWarning(log) << "Failed to kick off login flow v2";
        return false;
    }
}

/**
 * @brief Constructor.
 */
NextCloudLoginFlow::NextCloudLoginFlow(NextCloudLoginFlowPrivate* d, QObject* parent)
    : QObject(parent), d_ptr(d)
{
}

} // namespace SynqClient
