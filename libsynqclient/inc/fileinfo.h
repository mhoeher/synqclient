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

#ifndef SYNQCLIENT_FILEINFO_H
#define SYNQCLIENT_FILEINFO_H

#include <QObject>
#include <QSharedDataPointer>
#include <QVector>
#include <QtGlobal>

#include "libsynqclient_global.h"

namespace SynqClient {

class FileInfoPrivate;

class LIBSYNQCLIENT_EXPORT FileInfo
{
public:
    FileInfo();
    FileInfo(const FileInfo& other);
    virtual ~FileInfo();
    FileInfo& operator=(const FileInfo& other);

    bool isValid() const;

    bool isFile() const;
    void setIsFile();

    bool isDirectory() const;
    void setIsDirectory();

    QString name() const;
    void setName(const QString& name);

    QString syncAttribute() const;
    void setSyncAttribute(const QString& syncAttribute);

    QUrl url() const;
    void setUrl(const QUrl& url);

    QVariant customProperty(const QString& name) const;
    void setCustomProperty(const QString& name, const QVariant& propertyValue);

    static FileInfo fromLocalFile(const QString& path);

protected:
    explicit FileInfo(FileInfoPrivate* d);

    QSharedDataPointer<FileInfoPrivate> d;
};

typedef QVector<FileInfo> FileInfos;

} // namespace SynqClient

#endif // SYNQCLIENT_FILEINFO_H
