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

#ifndef SYNQCLIENT_WEBDAVDOWNLOADFILEJOBPRIVATE_H
#define SYNQCLIENT_WEBDAVDOWNLOADFILEJOBPRIVATE_H

#include "downloadfilejobprivate.h"
#include "SynqClient/webdavdownloadfilejob.h"

namespace SynqClient {

class WebDAVDownloadFileJobPrivate : public DownloadFileJobPrivate
{
public:
    explicit WebDAVDownloadFileJobPrivate(WebDAVDownloadFileJob* q);

    Q_DECLARE_PUBLIC(WebDAVDownloadFileJob);

    QIODevice* downloadDevice;

    void checkParameters();
    void handleRequestFinished();
};

} // namespace SynqClient

#endif // SYNQCLIENT_WEBDAVDOWNLOADFILEJOBPRIVATE_H
