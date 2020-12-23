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

#include "../inc/listfilesjob.h"

#include "listfilesjobprivate.h"

namespace SynqClient {

ListFilesJob::ListFilesJob(QObject* parent) : AbstractJob(new ListFilesJobPrivate(this), parent) {}

ListFilesJob::~ListFilesJob() {}

QString ListFilesJob::path() const
{
    Q_D(const ListFilesJob);
    return d->path;
}

void ListFilesJob::setPath(const QString& path)
{
    Q_D(ListFilesJob);
    d->path = path;
}

FileInfos ListFilesJob::entries() const
{
    Q_D(const ListFilesJob);
    return d->entries;
}

ListFilesJob::ListFilesJob(ListFilesJobPrivate* d, QObject* parent) : AbstractJob(d, parent) {}

void ListFilesJob::setEntries(const FileInfos& entries)
{
    Q_D(ListFilesJob);
    d->entries = entries;
}

} // namespace SynqClient
