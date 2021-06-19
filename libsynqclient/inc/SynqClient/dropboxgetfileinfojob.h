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

#ifndef SYNQCLIENT_DROPBOXGETFILEINFOJOB_H
#define SYNQCLIENT_DROPBOXGETFILEINFOJOB_H

#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>

#include "SynqClient/AbstractDropboxJob"
#include "SynqClient/GetFileInfoJob"
#include "SynqClient/libsynqclient_global.h"

namespace SynqClient {

class DropboxGetFileInfoJobPrivate;

class LIBSYNQCLIENT_EXPORT DropboxGetFileInfoJob : public GetFileInfoJob, public AbstractDropboxJob
{
    Q_OBJECT
public:
    explicit DropboxGetFileInfoJob(QObject* parent = nullptr);
    ~DropboxGetFileInfoJob() override;

    // AbstractJob interface
    void start() override;
    void stop() override;

protected:
    explicit DropboxGetFileInfoJob(DropboxGetFileInfoJobPrivate* d, QObject* parent = nullptr);

    Q_DECLARE_PRIVATE(DropboxGetFileInfoJob);
};

} // namespace SynqClient

#endif // SYNQCLIENT_DROPBOXGETFILEINFOJOB_H
