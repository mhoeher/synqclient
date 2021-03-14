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

#ifndef SYNQCLIENT_GETFILEINFOJOB_H
#define SYNQCLIENT_GETFILEINFOJOB_H

#include <QObject>
#include <QScopedPointer>
#include <QVariantMap>
#include <QtGlobal>

#include "AbstractJob"
#include "FileInfo"
#include "libsynqclient_global.h"

namespace SynqClient {

class GetFileInfoJobPrivate;

class LIBSYNQCLIENT_EXPORT GetFileInfoJob : public AbstractJob
{
    Q_OBJECT
public:
    explicit GetFileInfoJob(QObject* parent = nullptr);
    ~GetFileInfoJob() override;

    QString path() const;
    void setPath(const QString& path);

    FileInfo fileInfo() const;

protected:
    explicit GetFileInfoJob(GetFileInfoJobPrivate* d, QObject* parent = nullptr);

    Q_DECLARE_PRIVATE(GetFileInfoJob);

    void setFileInfo(const FileInfo& fileInfo);
};

} // namespace SynqClient

#endif // SYNQCLIENT_GETFILEINFOJOB_H
