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

#include "SynqClient/fileinfo.h"

#include "fileinfoprivate.h"

#include <QFileInfo>

namespace SynqClient {

/**
 * @class FileInfo
 * @brief Meta information about a file or folder.
 *
 * This class is used to provide meta information about a file or folder. Usually, it is used by
 * jobs which (also) retrieve meta information about remote resources, e.g. GetFileInfoJob() or
 * ListFilesJob().
 */

/**
 * @brief Constructor.
 *
 * This creates a new, invalid FileInfo object.
 *
 * @sa isValid()
 */
FileInfo::FileInfo() : d(new FileInfoPrivate) {}

/**
 * @brief Copy constructor.
 */
FileInfo::FileInfo(const FileInfo& other) : d(other.d) {}

/**
 * @brief Destructor.
 */
FileInfo::~FileInfo() {}

/**
 * @brief Copy assignment operator.
 */
FileInfo& FileInfo::operator=(const FileInfo& other)
{
    d = other.d;
    return *this;
}

/**
 * @brief Indicates if the object is valid.
 */
bool FileInfo::isValid() const
{
    return d->type != FileInfoPrivate::Invalid;
}

/**
 * @brief Indicates if the resource is a file.
 */
bool FileInfo::isFile() const
{
    return d->type == FileInfoPrivate::File;
}

/**
 * @brief Mark the resource as a file.
 *
 * This will mark this object as refering to a file.
 */
void FileInfo::setIsFile()
{
    d->type = FileInfoPrivate::File;
}

/**
 * @brief Indicates if the resource is a folder.
 */
bool FileInfo::isDirectory() const
{
    return d->type == FileInfoPrivate::Directory;
}

/**
 * @brief Mark this object as refering to a folder.
 */
void FileInfo::setIsDirectory()
{
    d->type = FileInfoPrivate::Directory;
}

/**
 * @brief The name of the file or folder.
 *
 * This holds the file name (i.e. without the path) of a file or folder.
 */
QString FileInfo::name() const
{
    return d->name;
}

/**
 * @brief Set the file name.
 */
void FileInfo::setName(const QString& name)
{
    d->name = name;
}

/**
 * @brief The remote sync attribute.
 *
 * This attribute can be used for synchronizing a local and remote version of a file or folder. The
 * sync attribute indicates the *version* of a file. It is a string, the exact content depends on
 * the concrete backend. Sync attributes are used for:
 *
 * - Detecting if a file changed.
 * - Detecting if a folder (e.g. the contents of the folder) changed, in other words: If there have
 * been any creations, updates or deletes of entries inside this folder. In this case, the sync
 * attribute of the folder shall change as well.
 *
 * If no sync attribute could be retrieved, this will be an empty string.
 */
QString FileInfo::syncAttribute() const
{
    return d->syncAttribute;
}

/**
 * @brief Set the sync attribute.
 */
void FileInfo::setSyncAttribute(const QString& syncAttribute)
{
    d->syncAttribute = syncAttribute;
}

/**
 * @brief The full URL to the remote file.
 *
 * If this object refers to a remote file or folder, this should hold the full URL to it.
 */
QUrl FileInfo::url() const
{
    return d->url;
}

/**
 * @brief Set the URL to the remote file or folder.
 */
void FileInfo::setUrl(const QUrl& url)
{
    d->url = url;
}

/**
 * @brief Retrieve custom properties called @p name.
 *
 * Depending on the concrete backend, additional meta information might be retrieved for a remote
 * file or folder. In this case, such information can be stored as custom properties in the form of
 * a key-value mapping. Keys are arbitrary strings up to the concrete backend. Values can be
 * anything. Refer to the documentation of the concrete jobs to learn about supported custom
 * properties.
 */
QVariant FileInfo::customProperty(const QString& name) const
{
    return d->customProperties.value(name);
}

/**
 * @brief Set the custom property @p name to @p value.
 */
void FileInfo::setCustomProperty(const QString& name, const QVariant& propertyValue)
{
    d->customProperties[name] = propertyValue;
}

/**
 * @brief Construct a FileInfo object from a local file.
 *
 * This is a utility method, which constructs a FileInfo object from the local file or folder
 * pointed to by the given @p path.
 */
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

/**
 * @brief Constructor.
 */
FileInfo::FileInfo(FileInfoPrivate* d) : d(d) {}

} // namespace SynqClient
