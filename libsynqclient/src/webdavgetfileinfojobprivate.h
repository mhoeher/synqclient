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

#ifndef SYNQCLIENT_WEBDAVGETFILEINFOJOBPRIVATE_H
#define SYNQCLIENT_WEBDAVGETFILEINFOJOBPRIVATE_H

#include <QString>

#include "webdavgetfileinfojob.h"
#include "getfileinfojobprivate.h"

namespace SynqClient {

class WebDAVGetFileInfoJobPrivate : public GetFileInfoJobPrivate
{
public:
    explicit WebDAVGetFileInfoJobPrivate(WebDAVGetFileInfoJob* q);

    Q_DECLARE_PUBLIC(WebDAVGetFileInfoJob);

    void checkParameters();
    void handleRequestFinished();
};

} // namespace SynqClient

#endif // SYNQCLIENT_WEBDAVGETFILEINFOJOBPRIVATE_H
