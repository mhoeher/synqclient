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

#ifndef SYNQCLIENT_ABSTRACTJOBFACTORY_H
#define SYNQCLIENT_ABSTRACTJOBFACTORY_H

#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>

#include "libsynqclient.h"
#include "libsynqclient_global.h"

namespace SynqClient {

class AbstractJob;
class CreateDirectoryJob;
class DeleteJob;
class DownloadFileJob;
class UploadFileJob;
class GetFileInfoJob;
class ListFilesJob;

class AbstractJobFactoryPrivate;

class LIBSYNQCLIENT_EXPORT AbstractJobFactory : public QObject
{
    Q_OBJECT
public:
    explicit AbstractJobFactory(QObject* parent = nullptr);
    ~AbstractJobFactory() override;

    CreateDirectoryJob* createDirectory(QObject* parent = nullptr);
    DeleteJob* deleteResource(QObject* parent = nullptr);
    DownloadFileJob* downloadFile(QObject* parent = nullptr);
    UploadFileJob* uploadFile(QObject* parent = nullptr);
    GetFileInfoJob* getFileInfo(QObject* parent = nullptr);
    ListFilesJob* listFiles(QObject* parent = nullptr);

    RemoteChangeDetectionMode remoteChangeDetectionMode() const;

protected:
    explicit AbstractJobFactory(AbstractJobFactoryPrivate* d, QObject* parent = nullptr);

    QScopedPointer<AbstractJobFactoryPrivate> d_ptr;
    Q_DECLARE_PRIVATE(AbstractJobFactory);

    /**
     * @brief Create a new job.
     *
     * Concrete factory classes must override this method to actually create
     * job objects. The @p type of job to create is passed in, as well as the
     * @p parent object which the resulting object shall belong to.
     *
     * On success, the created job shall be returned; on error, a
     * nullptr.
     */
    virtual AbstractJob* createJob(JobType type, QObject* parent = nullptr) = 0;

    void setRemoteChangeDetectionMode(RemoteChangeDetectionMode mode);
};

} // namespace SynqClient

#endif // SYNQCLIENT_ABSTRACTJOBFACTORY_H
