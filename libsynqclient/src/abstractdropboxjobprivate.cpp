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

#include "abstractwebdavjobprivate.h"

namespace SynqClient {

const QString AbstractDropboxJobPrivate::APIv2 = "https://api.dropboxapi.com/2";

AbstractDropboxJobPrivate::AbstractDropboxJobPrivate(AbstractDropboxJob* q)
    : q_ptr(q),
      networkAccessManager(nullptr),
      userAgent(AbstractWebDAVJobPrivate::DefaultUserAgent),
      token(),
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
 */
FileInfo AbstractDropboxJobPrivate::fileInfoFromJson(const QJsonObject& obj,
                                                     const QString& basePath)
{
    auto tag = obj.value(".tag").toString();
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
            result.setPath(QDir(basePath).relativeFilePath(obj.value("path_display").toString()));
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

    auto reply = networkAccessManager->post(req, QJsonDocument::fromVariant(data).toJson());

    return reply;
}

} // namespace SynqClient
