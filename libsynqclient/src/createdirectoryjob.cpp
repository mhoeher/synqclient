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

#include "../inc/createdirectoryjob.h"

#include "createdirectoryjobprivate.h"

namespace SynqClient {

CreateDirectoryJob::CreateDirectoryJob(QObject* parent)
    : AbstractJob(new CreateDirectoryJobPrivate(this), parent)
{
}

CreateDirectoryJob::~CreateDirectoryJob() {}

QString CreateDirectoryJob::path() const
{
    Q_D(const CreateDirectoryJob);
    return d->path;
}

void CreateDirectoryJob::setPath(const QString& path)
{
    Q_D(CreateDirectoryJob);
    d->path = path;
}

CreateDirectoryJob::CreateDirectoryJob(CreateDirectoryJobPrivate* d, QObject* parent)
    : AbstractJob(d, parent)
{
}

} // namespace SynqClient