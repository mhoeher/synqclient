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

#ifndef SYNQCLIENT_DROPBOXLISTFILESJOBPRIVATE_H
#define SYNQCLIENT_DROPBOXLISTFILESJOBPRIVATE_H

#include "listfilesjobprivate.h"
#include "SynqClient/dropboxlistfilesjob.h"

namespace SynqClient {

class DropboxListFilesJobPrivate : public ListFilesJobPrivate
{
public:
    explicit DropboxListFilesJobPrivate(DropboxListFilesJob* q);

    Q_DECLARE_PUBLIC(DropboxListFilesJob);

    QString cursor;
};

} // namespace SynqClient

#endif // SYNQCLIENT_DROPBOXLISTFILESJOBPRIVATE_H
