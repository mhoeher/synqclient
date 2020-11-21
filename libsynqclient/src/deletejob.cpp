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

#include "../inc/deletejob.h"

#include "deletejobprivate.h"

namespace SynqClient {

DeleteJob::DeleteJob(QObject* parent) : AbstractJob(new DeleteJobPrivate(this), parent) {}

DeleteJob::~DeleteJob() {}

QString DeleteJob::path() const
{
    Q_D(const DeleteJob);
    return d->path;
}

void DeleteJob::setPath(const QString& path)
{
    Q_D(DeleteJob);
    d->path = path;
}

/**
 * @brief Delete only if the remote file's sync attribute matches.
 *
 * If this property is set to a valid (i.e. non-null) FileProperty::SyncAttribute,
 * then the request shall only succeed if the remote file's sync attribute matches
 * the set one.
 *
 * @note This might or might not work, depending on the concrete protocol in
 * question.
 */
QVariant DeleteJob::syncAttribute() const
{
    Q_D(const DeleteJob);
    return d->syncAttribute;
}

/**
 * @brief Set the @p syncAttribute we expect on the remote file.
 */
void DeleteJob::setSyncAttribute(const QVariant& syncAttribute)
{
    Q_D(DeleteJob);
    d->syncAttribute = syncAttribute;
}

DeleteJob::DeleteJob(DeleteJobPrivate* d, QObject* parent) : AbstractJob(d, parent) {}

} // namespace SynqClient
