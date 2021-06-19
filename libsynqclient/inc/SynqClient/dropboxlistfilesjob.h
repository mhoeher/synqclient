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

#ifndef SYNQCLIENT_DROPBOXLISTFILESJOB_H
#define SYNQCLIENT_DROPBOXLISTFILESJOB_H

#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>

#include "SynqClient/ListFilesJob"
#include "SynqClient/libsynqclient_global.h"
#include "SynqClient/AbstractDropboxJob"

namespace SynqClient {

class DropboxListFilesJobPrivate;

class LIBSYNQCLIENT_EXPORT DropboxListFilesJob : public ListFilesJob, public AbstractDropboxJob
{
    Q_OBJECT
public:
    explicit DropboxListFilesJob(QObject* parent = nullptr);
    ~DropboxListFilesJob() override;

    // AbstractJob interface
    void start() override;
    void stop() override;

protected:
    explicit DropboxListFilesJob(DropboxListFilesJobPrivate* d, QObject* parent = nullptr);

    Q_DECLARE_PRIVATE(DropboxListFilesJob);
};

} // namespace SynqClient

#endif // SYNQCLIENT_DROPBOXLISTFILESJOB_H
