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

#include "../inc/fileinfo.h"

#include "fileinfoprivate.h"

#include <QFileInfo>

namespace SynqClient {

FileInfo::FileInfo() : d(new FileInfoPrivate) {}

FileInfo::FileInfo(const FileInfo& other) : d(other.d) {}

FileInfo::~FileInfo() {}

FileInfo& FileInfo::operator=(const FileInfo& other)
{
    d = other.d;
    return *this;
}

bool FileInfo::isValid() const
{
    return d->type != FileInfoPrivate::Invalid;
}

bool FileInfo::isFile() const
{
    return d->type == FileInfoPrivate::File;
}

void FileInfo::setIsFile()
{
    d->type = FileInfoPrivate::File;
}

bool FileInfo::isDirectory() const
{
    return d->type == FileInfoPrivate::Directory;
}

void FileInfo::setIsDirectory()
{
    d->type = FileInfoPrivate::Directory;
}

QString FileInfo::name() const
{
    return d->name;
}

void FileInfo::setName(const QString& name)
{
    d->name = name;
}

QString FileInfo::syncAttribute() const
{
    return d->syncAttribute;
}

void FileInfo::setSyncAttribute(const QString& syncAttribute)
{
    d->syncAttribute = syncAttribute;
}

QUrl FileInfo::url() const
{
    return d->url;
}

void FileInfo::setUrl(const QUrl& url)
{
    d->url = url;
}

QVariant FileInfo::customProperty(const QString& name) const
{
    return d->customProperties.value(name);
}

void FileInfo::setCustomProperty(const QString& name, const QVariant& propertyValue)
{
    d->customProperties[name] = propertyValue;
}

FileInfo FileInfo::fromLocalFile(const QString& path)
{
    FileInfo result;
    QFileInfo fi(path);
    if (fi.exists()) {
        if (fi.isFile()) {
            result.setIsFile();
        } else if (fi.isDir()) {
            result.setIsDirectory();
        }
        result.setName(fi.fileName());
    }
    return result;
}

FileInfo::FileInfo(FileInfoPrivate* d) : d(d) {}

} // namespace SynqClient
