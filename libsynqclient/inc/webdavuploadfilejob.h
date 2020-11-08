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

#ifndef SYNQCLIENT_WEBDAVUPLOADFILEJOB_H
#define SYNQCLIENT_WEBDAVUPLOADFILEJOB_H

#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>

#include "AbstractWebDAVJob"
#include "UploadFileJob"
#include "libsynqclient_global.h"

namespace SynqClient {

class WebDAVUploadFileJobPrivate;

class LIBSYNQCLIENT_EXPORT WebDAVUploadFileJob : public UploadFileJob, public AbstractWebDAVJob
{
    Q_OBJECT
public:
    explicit WebDAVUploadFileJob(QObject* parent = nullptr);
    ~WebDAVUploadFileJob() override;

    // AbstractJob interface
    void start() override;
    void stop() override;

protected:
    explicit WebDAVUploadFileJob(WebDAVUploadFileJobPrivate* d, QObject* parent = nullptr);

    Q_DECLARE_PRIVATE(WebDAVUploadFileJob);
};

} // namespace SynqClient

#endif // SYNQCLIENT_WEBDAVUPLOADFILEJOB_H
