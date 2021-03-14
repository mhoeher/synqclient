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

#ifndef SYNQCLIENT_CREATEDIRECTORYJOB_H
#define SYNQCLIENT_CREATEDIRECTORYJOB_H

#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>

#include "AbstractJob"
#include "libsynqclient_global.h"

namespace SynqClient {

class CreateDirectoryJobPrivate;

class LIBSYNQCLIENT_EXPORT CreateDirectoryJob : public AbstractJob
{
    Q_OBJECT
public:
    explicit CreateDirectoryJob(QObject* parent = nullptr);
    ~CreateDirectoryJob() override;

    QString path() const;
    void setPath(const QString& path);

protected:
    explicit CreateDirectoryJob(CreateDirectoryJobPrivate* d, QObject* parent = nullptr);

    Q_DECLARE_PRIVATE(CreateDirectoryJob);
};

} // namespace SynqClient

#endif // SYNQCLIENT_CREATEDIRECTORYJOB_H
