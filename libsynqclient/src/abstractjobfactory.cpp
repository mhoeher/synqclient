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

#include "SynqClient/abstractjobfactory.h"

#include "abstractjobfactoryprivate.h"
#include "SynqClient/abstractjob.h"
#include "SynqClient/createdirectoryjob.h"
#include "SynqClient/deletejob.h"
#include "SynqClient/downloadfilejob.h"
#include "SynqClient/uploadfilejob.h"
#include "SynqClient/getfileinfojob.h"
#include "SynqClient/listfilesjob.h"

namespace SynqClient {

template<typename T>
static T* checkJob(AbstractJob* job)
{
    auto result = qobject_cast<T*>(job);
    if (result) {
        return result;
    } else {
        delete job;
        return nullptr;
    }
}

/**
 * @class AbstractJobFactory
 * @brief Factory for jobs.
 *
 * This class is the abstract base for concrete factories which produce jobs
 * that can be used for a concrete protocol or server. On is own, this class
 * does nothing (but provide a nice interface around the virtual part
 * of the class).
 *
 * Concrete sub-classes must implement the interface defined by this class
 * in order to allow working with a specific protocol or backend server.
 */
/**
 * @brief Constructor.
 */
AbstractJobFactory::AbstractJobFactory(QObject* parent)
    : QObject(parent), d_ptr(new AbstractJobFactoryPrivate(this))
{
}

/**
 * @brief Destructor.
 */
AbstractJobFactory::~AbstractJobFactory() {}

/**
 * @brief Create a job to create a new folder.
 *
 * This creates a job which creates a folder remotely. The resulting object
 * will be owned by the @p parent. If creating the job fails, a nullptr is
 * returned.
 */
CreateDirectoryJob* AbstractJobFactory::createDirectory(QObject* parent)
{
    return checkJob<CreateDirectoryJob>(createJob(JobType::CreateDirectory, parent));
}

/**
 * @brief Create a job to delete a file or directory.
 *
 * This creates a job which deletes resources remotely. The resulting object
 * will be owned by the @p parent. If creating the job fails, a nullptr is
 * returned.
 */
DeleteJob* AbstractJobFactory::deleteResource(QObject* parent)
{
    return checkJob<DeleteJob>(createJob(JobType::DeleteResource, parent));
}

/**
 * @brief Create a job to download a file.
 *
 * This creates a job which downloads a file. The resulting object
 * will be owned by the @p parent. If creating the job fails, a nullptr is
 * returned.
 */
DownloadFileJob* AbstractJobFactory::downloadFile(QObject* parent)
{
    return checkJob<DownloadFileJob>(createJob(JobType::DownloadFile, parent));
}

/**
 * @brief Create a job to upload a file.
 *
 * This creates a job which upload a file. The resulting object
 * will be owned by the @p parent. If creating the job fails, a nullptr is
 * returned.
 */
UploadFileJob* AbstractJobFactory::uploadFile(QObject* parent)
{
    return checkJob<UploadFileJob>(createJob(JobType::UploadFile, parent));
}

/**
 * @brief Create a job to fetch information about a single file or folder.
 *
 * This creates a job which gets information about a single remote file or
 * directory. The resulting object
 * will be owned by the @p parent. If creating the job fails, a nullptr is
 * returned.
 */
GetFileInfoJob* AbstractJobFactory::getFileInfo(QObject* parent)
{
    return checkJob<GetFileInfoJob>(createJob(JobType::GetFileInfo, parent));
}

/**
 * @brief Create a job to list remote files.
 *
 * This creates a job which lists the entries in a remote directory.
 * The resulting object
 * will be owned by the @p parent. If creating the job fails, a nullptr is
 * returned.
 */
ListFilesJob* AbstractJobFactory::listFiles(QObject* parent)
{
    return checkJob<ListFilesJob>(createJob(JobType::ListFiles, parent));
}

/**
 * @brief Returns the mode that is used to detect remote changes.
 *
 * This returns the mode that is used for the backend represented by the factory to detect changes
 * on the remote server.
 *
 * @sa setRemoteChangeDetectionMode
 */
RemoteChangeDetectionMode AbstractJobFactory::remoteChangeDetectionMode() const
{
    Q_D(const AbstractJobFactory);
    return d->syncDetectionMode;
}

/**
 * @brief Does a sync always need to follow sub-folders?
 *
 * This property indicates if a sync needs to always check all subfolders for detecting changes.
 * @sa setAlwaysCheckSubfolders
 */
bool AbstractJobFactory::alwaysCheckSubfolders() const
{
    Q_D(const AbstractJobFactory);
    return d->alwaysCheckSubfolders;
}

/**
 * @brief Constructor.
 */
AbstractJobFactory::AbstractJobFactory(AbstractJobFactoryPrivate* d, QObject* parent)
    : QObject(parent), d_ptr(d)
{
}

/**
 * @brief Set the mode used to detect remote changes during synchronization.
 *
 * This sets the mode that is used by synchronization code to detect changes on the remote server.
 * Different backends behave differently, so the way remote changes are detected efficiently might
 * require backend specific handling.
 *
 * The default value is RemoteChangeDetectionMode::FoldersWithSyncAttributes, but concrete factories
 * might call this in their constructor to change the mode something else if the backend they
 * represent need specific handling.
 *
 * Note that the default mode should in general work with any backend, however, a more efficient
 * handling (usually resulting in fewer queries that need to be run) might be possible by changing
 * to a different mode.
 */
void AbstractJobFactory::setRemoteChangeDetectionMode(RemoteChangeDetectionMode mode)
{
    Q_D(AbstractJobFactory);
    d->syncDetectionMode = mode;
}

/**
 * @brief Set if a sync shall always check subfolders.
 *
 * This method can be used to indicate to a synchronization mechanism such as DirectorySynchronizer,
 * that subfolders always have to be checked. For example, when using WebDAV style sync, usually,
 * sync attributes are assumed to change if there is a change either directly or recursively the
 * sync attribute of a folder changes. If this flag is set to true, subfolders will always be
 * followed and checked for changes, even if the server reports unchanged sync attributes.
 */
void AbstractJobFactory::setAlwaysCheckSubfolders(bool alwaysCheckSubfolders)
{
    Q_D(AbstractJobFactory);
    d->alwaysCheckSubfolders = alwaysCheckSubfolders;
}

} // namespace SynqClient
