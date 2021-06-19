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

#include "abstractdropboxjobprivate.h"

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>

#include <cmath>

#include "abstractwebdavjobprivate.h"

namespace SynqClient {

static Q_LOGGING_CATEGORY(log, "SynqClient.AbstractDropboxJob", QtWarningMsg);

const QString AbstractDropboxJobPrivate::APIv2 = "https://api.dropboxapi.com/2";
const QString AbstractDropboxJobPrivate::ContentAPIv2 = "https://content.dropboxapi.com/2";

AbstractDropboxJobPrivate::AbstractDropboxJobPrivate(AbstractDropboxJob* q)
    : q_ptr(q),
      networkAccessManager(nullptr),
      userAgent(AbstractWebDAVJobPrivate::DefaultUserAgent),
      token(),
      numRetries(0),
      reply(nullptr)
{
}

AbstractDropboxJobPrivate::~AbstractDropboxJobPrivate()
{
    if (reply) {
        reply->deleteLater();
        reply.clear();
    }
}

std::tuple<JobError, QString> AbstractDropboxJobPrivate::checkDefaultParameters()
{
    if (networkAccessManager == nullptr) {
        return std::make_tuple(JobError::MissingParameter, tr("No QNetworkAccessManager is set"));
    }

    if (userAgent.isEmpty()) {
        return std::make_tuple(JobError::MissingParameter, tr("No user agent string is set"));
    }

    if (token.isEmpty()) {
        return std::make_tuple(JobError::MissingParameter, tr("Missing bearer token"));
    }

    return std::make_tuple(JobError::NoError, QString());
}

/**
 * @brief Converts a Dropbox JSON response to a FileInfo object.
 *
 * This takes a JSON object which represents a Dropbox FileMetadata, FolderMetadata or
 * DeletedMetadata and converts it to a suitable FileInfo object.
 *
 * If @p basePath is set to a non-null string, the FileInfo::path() will be set to the
 * path of the object, relative to the base path. Otherwise, the path will not be set at all.
 *
 * If @p forceTag is set to a non-null string, assume that the metadata refers to an object of the
 * givenm type (e.g. file or folder). Otherwise, the type is read from the JSON document.
 */
FileInfo AbstractDropboxJobPrivate::fileInfoFromJson(const QJsonObject& obj,
                                                     const QString& basePath,
                                                     const QString& forceTag)
{
    auto tag = forceTag;
    if (tag.isNull()) {
        tag = obj.value(".tag").toString();
    }
    FileInfo result;
    if (tag == "file") {
        result.setIsFile();
        result.setSyncAttribute(obj.value("rev").toString());
    } else if (tag == "folder") {
        result.setIsDirectory();
    } else if (tag == "deleted") {
        result.setDeleted(true);
    }

    if (result.isValid() || result.isDeleted()) {
        result.setName(obj.value("name").toString());
        if (!basePath.isNull()) {
            result.setPath(
                    QDir(fixPath(basePath)).relativeFilePath(obj.value("path_display").toString()));
        }
        result.setCustomProperty(AbstractDropboxJob::DropboxFileInfoKey, obj.toVariantMap());
    }
    return result;
}

QNetworkReply* AbstractDropboxJobPrivate::post(const QString& endpoint, const QVariant& data)
{
    QNetworkRequest req;
    req.setUrl(AbstractDropboxJobPrivate::APIv2 + endpoint);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", "Bearer " + token.toUtf8());

    auto reply = networkAccessManager->post(
            req, QJsonDocument::fromVariant(data).toJson(QJsonDocument::Compact));

    return reply;
}

QNetworkReply* AbstractDropboxJobPrivate::postData(const QString& endpoint, const QVariant& data,
                                                   QIODevice* content)
{
    QNetworkRequest req;
    req.setUrl(AbstractDropboxJobPrivate::ContentAPIv2 + endpoint);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
    req.setRawHeader("Authorization", "Bearer " + token.toUtf8());
    req.setRawHeader("Dropbox-API-Arg",
                     QJsonDocument::fromVariant(data).toJson(QJsonDocument::Compact));

    auto reply = networkAccessManager->post(req, content);

    return reply;
}

/**
 * @brief Helper function to handle Dropbox error results.
 *
 * When a Dropbox API request fails, the network reply finishes with an error. We can try to
 * examine the response body. If it is a "known" Dropbox issue, a JSON object will be returned.
 *
 * This function can be used to unify handling of such errors. It takes the response body as
 * well as a map of key value pairs, where:
 *
 * - Keys are pairs of a list of members (in hierarchical order) forming a path in the JSON
 *   document and the value being the string value to compare against.
 * - The value of the map is used, which is a callable taking the QJsonDocument as argument. The
 *   callable can run any code required (e.g. set proper error codes). Afterwards, the error
 * handing stops.
 */
void AbstractDropboxJobPrivate::tryHandleKnownError(
        const QByteArray& body,
        QMap<QPair<QStringList, QVariant>, KnownErrorHandlerFunction> handlers)
{
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(body, &error);
    if (error.error == QJsonParseError::NoError) {
        for (auto it = handlers.constBegin(); it != handlers.constEnd(); ++it) {
            auto parts = it.key().first;
            QJsonValue val = doc.object();
            for (const auto& part : qAsConst(parts)) {
                val = val.toObject().value(part);
                if (val.isUndefined()) {
                    break;
                }
            }
            if (val.isString()) {
                if (val.toVariant() == it.key().second) {
                    auto handler = it.value();
                    if (handler) {
                        handler(doc);
                    }
                    return;
                }
            }
        }
    }
}

/**
 * @brief Fix a remote path.
 *
 * This makes sure that the @p path (which refers to a remote file or folder) starts with a slash.
 */
QString AbstractDropboxJobPrivate::fixPath(const QString& path)
{
    auto p = QDir::cleanPath(path);
    if (!p.startsWith("/")) {
        return "/" + p;
    } else {
        return p;
    }
}

bool AbstractDropboxJobPrivate::checkIfRequestShallBeRetried(QNetworkReply* reply) const
{
    if (reply && reply->error() != QNetworkReply::NoError && numRetries < MaxRetries) {
        auto code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (code == 429) {
            qCDebug(log) << "Server replied with code 429 (Too Many Requests) - retrying";
            return true;
        }
    }
    return false;
}

int AbstractDropboxJobPrivate::getRetryDelayInMilliseconds(QNetworkReply* reply) const
{
    int result = 0;
    if (reply) {
        bool ok;
        // Check if Retry-After is set and contains an integer value. In this case, it
        // is a delay to wait (in seconds), see
        // https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Retry-After.
        // According to the Dropbox API docs, the alternate datetime response is not used, so no
        // reason to check for it here.
        result = reply->rawHeader("Retry-After").toInt(&ok) * 1000;
        qCDebug(log) << "Server provided retry delay of" << result << "ms";
        if (!ok) {
            result = 0;
        }
    }
    if (result == 0) {
        result = 5000;
    }
    result = static_cast<int>(result * std::pow(1.2, numRetries));
    qCDebug(log) << "Calculated retry delay is" << result;
    return result;
}

} // namespace SynqClient
