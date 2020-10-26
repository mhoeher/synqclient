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

#include "../inc/getfileinfojob.h"

#include "getfileinfojobprivate.h"

namespace SynqClient {

GetFileInfoJob::GetFileInfoJob(QObject* parent)
    : AbstractJob(new GetFileInfoJobPrivate(this), parent)
{
}

GetFileInfoJob::~GetFileInfoJob() {}

QString GetFileInfoJob::path() const
{
    Q_D(const GetFileInfoJob);
    return d->path;
}

void GetFileInfoJob::setPath(const QString& path)
{
    Q_D(GetFileInfoJob);
    d->path = path;
}

QVariantMap GetFileInfoJob::fileInfo() const
{
    Q_D(const GetFileInfoJob);
    return d->fileInfo;
}

GetFileInfoJob::GetFileInfoJob(GetFileInfoJobPrivate* d, QObject* parent) : AbstractJob(d, parent)
{
}

} // namespace SynqClient
