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

#include "SynqClient/webdavjobfactory.h"

#include <QUuid>

#include "webdavjobfactoryprivate.h"

#include "SynqClient/webdavcreatedirectoryjob.h"
#include "SynqClient/webdavdeletejob.h"
#include "SynqClient/webdavdownloadfilejob.h"
#include "SynqClient/webdavuploadfilejob.h"
#include "SynqClient/webdavgetfileinfojob.h"
#include "SynqClient/webdavlistfilesjob.h"

namespace SynqClient {

/**
 * @class WebDAVJobFactory
 * @brief Create jobs to talk to a WebDAV server.
 *
 * This factory is used to create jobs which can be used to talk with a
 * concrete WebDAV server. The factory needs to be configured: At least a
 * networkAccessManager() must be set as well as the base url() of the server
 * to talk to.
 */

/**
 * @brief Constructor.
 */
WebDAVJobFactory::WebDAVJobFactory(QObject* parent)
    : AbstractJobFactory(new WebDAVJobFactoryPrivate(this), parent)
{
    // By default, do not follow subfolders unless their ETags change:
    setAlwaysCheckSubfolders(false);

    // Detect changes by checking sync attributes of folders:
    setRemoteChangeDetectionMode(RemoteChangeDetectionMode::FoldersWithSyncAttributes);
}

/**
 * @brief Destructor.
 */
WebDAVJobFactory::~WebDAVJobFactory() {}

/**
 * @brief The network access manager used by jobs created by the factory.
 */
QNetworkAccessManager* WebDAVJobFactory::networkAccessManager() const
{
    Q_D(const WebDAVJobFactory);
    return d->networkAccessManager;
}

/**
 * @brief Set the network access manager to be used by created jobs.
 */
void WebDAVJobFactory::setNetworkAccessManager(QNetworkAccessManager* networkAccessManager)
{
    Q_D(WebDAVJobFactory);
    d->networkAccessManager = networkAccessManager;
}

/**
 * @brief The base URL of the server to connect to.
 */
QUrl WebDAVJobFactory::url() const
{
    Q_D(const WebDAVJobFactory);
    return d->url;
}

/**
 * @brief Set the base URL of the server to connect to.
 */
void WebDAVJobFactory::setUrl(const QUrl& url)
{
    Q_D(WebDAVJobFactory);
    d->url = url;
}

/**
 * @brief The default user agent used by jobs created by the factory.
 */
QString WebDAVJobFactory::userAgent() const
{
    Q_D(const WebDAVJobFactory);
    return d->userAgent;
}

/**
 * @brief Set the default user agent for jobs created by the factory.
 */
void WebDAVJobFactory::setUserAgent(const QString& userAgent)
{
    Q_D(WebDAVJobFactory);
    d->userAgent = userAgent;
}

/**
 * @brief The type of WebDAV server to connect to.
 */
WebDAVServerType WebDAVJobFactory::serverType() const
{
    Q_D(const WebDAVJobFactory);
    return d->serverType;
}

/**
 * @brief Set the type of WebDAV server to connect to.
 */
void WebDAVJobFactory::setServerType(WebDAVServerType serverType)
{
    Q_D(WebDAVJobFactory);
    d->serverType = serverType;
}

/**
 * @brief Workarounds required for using the server.
 *
 * This property holds the required workarounds needed to work with the server. They can be manually
 * set or automatically retrieved using testServer().
 */
WebDAVWorkarounds WebDAVJobFactory::workarounds() const
{
    Q_D(const WebDAVJobFactory);
    return d->workarounds;
}

/**
 * @brief Set the required workarounds for the server.
 */
void WebDAVJobFactory::setWorkarounds(WebDAVWorkarounds workarounds)
{
    Q_D(WebDAVJobFactory);
    d->workarounds = workarounds;

    setAlwaysCheckSubfolders(workarounds.testFlag(WebDAVWorkaround::NoRecursiveFolderETags));
}

/**
 * @brief Test the server.
 *
 * This starts testing the server. WebDAV servers differ heavily in the way the implement the WebDAV
 * standard; some might even be unsuitable for running the synchronization.
 *
 * This function starts running some tests against the server, checking if it provides the necessary
 * functions that are needed to run a successful sync.
 *
 * Once the tests finish, the serverTestFinished() signal is emitted. The results of the tests are
 * stored in the workarounds() property and can be saved and later on restored.
 *
 * The optional @p path argument is the path on the server, where (temporary) files and folders
 * will be created in. Note that the remote path must exist, otherwise, the tests will fail.
 */
void WebDAVJobFactory::testServer(const QString& path)
{
    Q_D(WebDAVJobFactory);

    // If a test is already running, stop it:
    if (d->serverTestJob) {
        d->serverTestJob->stop();
        d->serverTestJob = nullptr;
    }

    // Start a new test sequence:
    auto testJob = new CompositeJob(this);
    testJob->setMaxJobs(1);
    testJob->setErrorMode(CompositeJobErrorMode::StopOnFirstError);

    auto uuid = QUuid::createUuid();
    auto rootPath = path + "/tmp-SynqClient-ServerTests-" + uuid.toString(QUuid::WithoutBraces);

    {
        // Create the remote folder.
        auto job = createDirectory(testJob);
        job->setPath(rootPath);
        testJob->addJob(job);
    }

    {
        // Create a sub-folder
        auto job = createDirectory(testJob);
        job->setPath(rootPath + "/sub-folder");
        testJob->addJob(job);
    }

    {
        // Get the ETag for the top level folder
        auto job = getFileInfo(testJob);
        job->setPath(rootPath);
        connect(job, &ListFilesJob::finished, this, [=]() {
            if (job->error() == JobError::NoError) {
                d->currentServerTestData["initialTopLevelETag"] = job->fileInfo().syncAttribute();
            }
        });
        testJob->addJob(job);
    }

    {
        // Upload a sample file
        auto job = uploadFile(testJob);
        job->setRemoteFilename(rootPath + "/sub-folder/test.txt");
        job->setData("Server Test");
        connect(job, &UploadFileJob::finished, this, [=]() {
            if (job->error() == JobError::NoError) {
                d->currentServerTestData["fileETagOnUpload"] = job->fileInfo().syncAttribute();
            }
        });
        testJob->addJob(job);
    }

    {
        // Download the sample file again
        auto job = downloadFile(testJob);
        job->setRemoteFilename(rootPath + "/sub-folder/test.txt");
        connect(job, &DownloadFileJob::finished, this, [=]() {
            d->currentServerTestData["fileETagOnDownload"] = job->fileInfo().syncAttribute();
        });
        testJob->addJob(job);
    }

    {
        // Get the ETag for the top level folder after the change
        auto job = getFileInfo(testJob);
        job->setPath(rootPath);
        connect(job, &ListFilesJob::finished, this, [=]() {
            if (job->error() == JobError::NoError) {
                d->currentServerTestData["changedTopLevelETag"] = job->fileInfo().syncAttribute();
            }
        });
        testJob->addJob(job);
    }

    {
        // Get the file ETag via PROPFIND:
        auto job = getFileInfo(testJob);
        job->setPath(rootPath + "/sub-folder/test.txt");
        connect(job, &ListFilesJob::finished, this, [=]() {
            if (job->error() == JobError::NoError) {
                d->currentServerTestData["fileETagViaPROPFIND"] = job->fileInfo().syncAttribute();
            }
        });
        testJob->addJob(job);
    }

    {
        // Delete the remote folder
        auto job = deleteResource(testJob);
        job->setPath(rootPath);
        testJob->addJob(job);
    }

    d->currentServerTestData.clear();
    d->serverTestJob = testJob;

    connect(testJob, &CompositeJob::finished, this, [=]() {
        testJob->deleteLater();
        if (testJob->error() == JobError::NoError) {
            WebDAVWorkarounds workarounds = WebDAVWorkaround::NoWorkarounds;
            workarounds = WebDAVWorkaround::NoWorkarounds;
            auto data = d->currentServerTestData;

            // Did the ETag of the top level folder change after the file change?
            if (data.contains("initialTopLevelETag") && data.contains("changedTopLevelETag")
                && data["initialTopLevelETag"] == data["changedTopLevelETag"]) {
                workarounds |= WebDAVWorkaround::NoRecursiveFolderETags;
            }

            // Does the server use different ETag formats when using GET and PROPFIND?
            if (data.contains("fileETagOnDownload") && data.contains("fileETagViaPROPFIND")) {
                auto fileETagOnDownload = data["fileETagOnDownload"].toString();
                auto fileETagViaPROPFIND = data["fileETagViaPROPFIND"].toString();
                if (fileETagOnDownload != fileETagViaPROPFIND) {
                    workarounds |= WebDAVWorkaround::InconsistentETagsUsingPROPFINDAndGET;
                }
                // Check if we can derive the PROPFIND ETag from the GET ETag (i.e.
                // the GET etag looks like "xxxx-y-zzz" and the PROPFIND one looks like
                // "y-zzz".
                auto propfindETagParts = fileETagViaPROPFIND.split("-");
                auto getETagParts = fileETagOnDownload.split("-");
                if (propfindETagParts.length() == 2 && getETagParts.length() == 3
                    && propfindETagParts[0] == getETagParts[1]
                    && propfindETagParts[1] == getETagParts[2]) {
                    workarounds |= WebDAVWorkaround::DerivePROPFINDETagsFromGETETagsForApache;
                }
            }

            setWorkarounds(workarounds);
        }
        emit serverTestFinished(testJob->error() == JobError::NoError);
    });
    connect(testJob, &CompositeJob::finished, testJob, &QObject::deleteLater);

    testJob->start();
}

/**
 * @brief Constructor.
 */
WebDAVJobFactory::WebDAVJobFactory(WebDAVJobFactoryPrivate* d, QObject* parent)
    : AbstractJobFactory(d, parent)
{
}

/**
 * @brief Implementation of AbstractJobFactory::createJob().
 */
AbstractJob* WebDAVJobFactory::createJob(JobType type, QObject* parent)
{
    Q_D(WebDAVJobFactory);
    switch (type) {
    case JobType::CreateDirectory:
        return d->createJob<WebDAVCreateDirectoryJob>(parent);
    case JobType::DeleteResource:
        return d->createJob<WebDAVDeleteJob>(parent);
    case JobType::DownloadFile:
        return d->createJob<WebDAVDownloadFileJob>(parent);
    case JobType::UploadFile:
        return d->createJob<WebDAVUploadFileJob>(parent);
    case JobType::GetFileInfo:
        return d->createJob<WebDAVGetFileInfoJob>(parent);
    case JobType::ListFiles:
        return d->createJob<WebDAVListFilesJob>(parent);
    default:
        return nullptr;
    }
}

} // namespace SynqClient
