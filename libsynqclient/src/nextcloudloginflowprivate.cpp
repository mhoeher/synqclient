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

#include "nextcloudloginflowprivate.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QNetworkReply>
#include <QTimer>
#include <QUrlQuery>

namespace SynqClient {

static Q_LOGGING_CATEGORY(log, "SynqClient.NextCloudLoginFlow", QtWarningMsg);

NextCloudLoginFlowPrivate::NextCloudLoginFlowPrivate(NextCloudLoginFlow* q)
    : QObject(),
      q_ptr(q),
      networkAccessManager(nullptr),
      pollTimer(nullptr),
      token(),
      endpoint(),
      userAgent("SynqClient"),
      flowRunning(false)
{
}

bool NextCloudLoginFlowPrivate::isValidFlowKickoffResponse(const QJsonDocument& doc, QString& error)
{
    if (!doc.isObject()) {
        error = "Response should be an object";
        return false;
    }
    auto obj = doc.object();
    if (!obj.contains("login")) {
        error = "Response does not contain a login URL";
        return false;
    }
    if (!obj.value("login").isString()) {
        error = "Login URL is not a string";
        return false;
    }
    if (!obj.contains("poll")) {
        error = "Response does not contain flow polling information";
        return false;
    }
    if (!obj.value("poll").isObject()) {
        error = "Poll information is not an object";
        return false;
    }
    auto poll = obj.value("poll").toObject();
    if (!poll.contains("token")) {
        error = "Poll information is missing token";
        return false;
    }
    if (!poll.value("token").isString()) {
        error = "Poll token is not a string";
        return false;
    }
    if (!poll.contains("endpoint")) {
        error = "Poll endpoint is missing";
        return false;
    }
    if (!poll.value("endpoint").isString()) {
        error = "Poll endpoint is not a string";
        return false;
    }
    return true;
}

bool NextCloudLoginFlowPrivate::isValidPollResultResponse(const QJsonDocument& doc, QString& error)
{
    if (!doc.isObject()) {
        error = "Poll result must be an object";
        return false;
    }
    auto obj = doc.object();
    if (!obj.contains("server")) {
        error = "Poll result must contain the server URL";
        return false;
    }
    if (!obj.contains("loginName")) {
        error = "Poll result must contain the login name";
        return false;
    }
    if (!obj.contains("appPassword")) {
        error = "Poll result must contain the app password";
        return false;
    }
    return true;
}

void NextCloudLoginFlowPrivate::onFlowKickoffFinished()
{
    Q_Q(NextCloudLoginFlow);
    auto reply = qobject_cast<QNetworkReply*>(sender());
    if (reply) {
        if (reply->error() != QNetworkReply::NoError) {
            qCWarning(log) << "Error during login flow kick off:" << reply->errorString();
            finish();
        } else {
            auto data = reply->readAll();
            QJsonParseError parseError;
            auto doc = QJsonDocument::fromJson(data, &parseError);
            if (parseError.error != QJsonParseError::NoError) {
                qCWarning(log) << "Invalid JSON data:" << parseError.errorString();
                finish();
            } else {
                QString validationError;
                if (!isValidFlowKickoffResponse(doc, validationError)) {
                    qCWarning(log) << "Invalid JSON data:" << validationError;
                    finish();
                } else {
                    auto login = doc.object().value("login").toString();
                    emit q->receivedLoginUrl(login);
                    token = doc.object().value("poll").toObject().value("token").toString();
                    endpoint = doc.object().value("poll").toObject().value("endpoint").toString();
                    poll();
                    return;
                }
            }
        }
    } else {
        qCWarning(log) << "onFlowKickoffFinished called from something else than a QNetworkReply";
    }
}

void NextCloudLoginFlowPrivate::onPollFinished()
{
    auto reply = qobject_cast<QNetworkReply*>(sender());
    if (reply) {
        switch (reply->error()) {
        case QNetworkReply::NoError:
            parsePollResult(reply->readAll());
            break;
        case QNetworkReply::ContentNotFoundError:
            // Expected, wait for some time and retry:
            if (!pollTimer) {
                pollTimer = new QTimer(this);
                pollTimer->setInterval(1000);
                pollTimer->setSingleShot(true);
                connect(pollTimer, &QTimer::timeout, this, &NextCloudLoginFlowPrivate::poll);
            }
            pollTimer->start();
            break;
        default:
            qCWarning(log) << "Unexpected network return code from polling endpoint:"
                           << reply->errorString() << "(" << reply->error() << ","
                           << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) << ")";
            finish();
            break;
        }
    } else {
        qCWarning(log) << "onPollFinished called from something else than a QNetworkReply";
    }
}

void NextCloudLoginFlowPrivate::poll()
{
    if (networkAccessManager) {
        QUrlQuery query;
        query.addQueryItem("token", token);
        QNetworkRequest request;
        request.setUrl(endpoint);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        auto reply = networkAccessManager->post(request, query.toString().toUtf8());
        if (!reply) {
            qCWarning(log) << "Failed to create poll reply";
            finish();
        } else {
            connect(reply, &QNetworkReply::finished, this,
                    &NextCloudLoginFlowPrivate::onPollFinished);
        }
    } else {
        qCWarning(log) << "poll called without flow ongoing";
    }
}

void NextCloudLoginFlowPrivate::parsePollResult(const QByteArray& data)
{
    Q_Q(NextCloudLoginFlow);
    QJsonParseError parseError;
    auto doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error == QJsonParseError::NoError) {
        QString validationError;
        if (isValidPollResultResponse(doc, validationError)) {
            auto obj = doc.object();
            emit q->receivedLogin(obj.value("loginName").toString(),
                                  obj.value("appPassword").toString(),
                                  QUrl(obj.value("server").toString()));
            qCDebug(log) << "Successfully finished login flow";
            finish();
        } else {
            qCWarning(log) << "Invalid poll result received from server:" << validationError;
            finish();
        }
    } else {
        qCWarning(log) << "Invalid result returned from poll endpoint:" << parseError.errorString();
        finish();
    }
}

void NextCloudLoginFlowPrivate::finish()
{
    Q_Q(NextCloudLoginFlow);
    flowRunning = false;
    emit q->flowRunningChanged();
    emit q->finished();
}

} // namespace SynqClient
