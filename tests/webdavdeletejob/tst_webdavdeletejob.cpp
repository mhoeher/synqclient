#include <QtTest>

// add necessary includes here
#include "../shared/utils.h"
#include "WebDAVCreateDirectoryJob"
#include "WebDAVDeleteJob"
#include "WebDAVGetFileInfoJob"
#include "WebDAVListFilesJob"
#include "WebDAVUploadFileJob"

using SynqClient::JobError;
using SynqClient::WebDAVCreateDirectoryJob;
using SynqClient::WebDAVDeleteJob;
using SynqClient::WebDAVGetFileInfoJob;
using SynqClient::WebDAVListFilesJob;
using SynqClient::WebDAVUploadFileJob;

class WebDAVDeleteJobTest : public QObject
{
    Q_OBJECT

public:
    WebDAVDeleteJobTest();
    ~WebDAVDeleteJobTest();

private slots:
    void initTestCase();
    void deleteSingleFile();
    void deleteSingleFile_data();
    void deleteEmptyFolder();
    void deleteEmptyFolder_data();
    void deleteFolderRecursively();
    void deleteFolderRecursively_data();
    void syncAttribute();
    void syncAttribute_data();
    void cleanupTestCase();
};

WebDAVDeleteJobTest::WebDAVDeleteJobTest() {}

WebDAVDeleteJobTest::~WebDAVDeleteJobTest() {}

void WebDAVDeleteJobTest::initTestCase() {}

void WebDAVDeleteJobTest::deleteSingleFile()
{
    QFETCH(QUrl, url);
    QFETCH(SynqClient::WebDAVServerType, type);

    QNetworkAccessManager nam;

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/WebDAVDeleteJobTest-deleteSingleFile-" + testDirUid.toString();
    auto remoteFileName = remotePath + "/hello.txt";

    {
        WebDAVCreateDirectoryJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setPath(remotePath);
        QSignalSpy spy(&job, &WebDAVCreateDirectoryJob::finished);
        job.start();
        QVERIFY(spy.wait());
    }

    {
        WebDAVUploadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setData("Hello World!\n");
        job.setRemoteFilename(remoteFileName);
        QSignalSpy spy(&job, &WebDAVUploadFileJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.errorString(), QString());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        WebDAVGetFileInfoJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setPath(remoteFileName);
        QSignalSpy spy(&job, &WebDAVGetFileInfoJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        WebDAVDeleteJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setPath(remoteFileName);
        QSignalSpy spy(&job, &WebDAVDeleteJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        WebDAVGetFileInfoJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setPath(remoteFileName);
        QSignalSpy spy(&job, &WebDAVGetFileInfoJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::ResourceNotFound);
    }
}

void WebDAVDeleteJobTest::deleteSingleFile_data()
{
    SynqClient::UnitTest::setupWebDAVTestServerData();
}

void WebDAVDeleteJobTest::deleteEmptyFolder()
{
    QFETCH(QUrl, url);
    QFETCH(SynqClient::WebDAVServerType, type);

    QNetworkAccessManager nam;

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/WebDAVDeleteJobTest-deleteEmptyFolder-" + testDirUid.toString();
    auto remoteFolderName = remotePath + "/hello";

    {
        WebDAVCreateDirectoryJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setPath(remotePath);
        QSignalSpy spy(&job, &WebDAVCreateDirectoryJob::finished);
        job.start();
        QVERIFY(spy.wait());
    }

    {
        WebDAVCreateDirectoryJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setPath(remoteFolderName);
        QSignalSpy spy(&job, &WebDAVCreateDirectoryJob::finished);
        job.start();
        QVERIFY(spy.wait());
    }

    {
        WebDAVGetFileInfoJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setPath(remoteFolderName);
        QSignalSpy spy(&job, &WebDAVGetFileInfoJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        WebDAVDeleteJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setPath(remoteFolderName);
        QSignalSpy spy(&job, &WebDAVDeleteJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        WebDAVGetFileInfoJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setPath(remoteFolderName);
        QSignalSpy spy(&job, &WebDAVGetFileInfoJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::ResourceNotFound);
    }
}

void WebDAVDeleteJobTest::deleteEmptyFolder_data()
{
    SynqClient::UnitTest::setupWebDAVTestServerData();
}

void WebDAVDeleteJobTest::deleteFolderRecursively()
{
    QFETCH(QUrl, url);
    QFETCH(SynqClient::WebDAVServerType, type);

    QNetworkAccessManager nam;

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/WebDAVDeleteJobTest-deleteFolderRecursively-" + testDirUid.toString();
    auto remoteSubFolder = remotePath + "/sub-folder";
    auto remoteFileName = remoteSubFolder + "/hello.txt";
    auto remoteFolderName = remoteSubFolder + "/dir";
    auto remoteFileInFolderName = remoteFolderName + "/hello2.txt";

    for (const auto& path : { remotePath, remoteSubFolder, remoteFolderName }) {
        WebDAVCreateDirectoryJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setPath(path);
        QSignalSpy spy(&job, &WebDAVCreateDirectoryJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.errorString(), QString());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        WebDAVUploadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setData("Hello World!\n");
        job.setRemoteFilename(remoteFileName);
        QSignalSpy spy(&job, &WebDAVUploadFileJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.errorString(), QString());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        WebDAVGetFileInfoJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setPath(remoteFileName);
        QSignalSpy spy(&job, &WebDAVGetFileInfoJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        WebDAVUploadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setData("Still there?\n");
        job.setRemoteFilename(remoteFileInFolderName);
        QSignalSpy spy(&job, &WebDAVUploadFileJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.errorString(), QString());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        WebDAVGetFileInfoJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setPath(remoteFileInFolderName);
        QSignalSpy spy(&job, &WebDAVGetFileInfoJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        WebDAVDeleteJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setPath(remoteSubFolder);
        QSignalSpy spy(&job, &WebDAVDeleteJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        WebDAVGetFileInfoJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setPath(remoteSubFolder);
        QSignalSpy spy(&job, &WebDAVGetFileInfoJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::ResourceNotFound);
    }

    {
        WebDAVListFilesJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setPath(remotePath);
        QSignalSpy spy(&job, &WebDAVListFilesJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.entries().length(), 0);
    }
}

void WebDAVDeleteJobTest::deleteFolderRecursively_data()
{
    SynqClient::UnitTest::setupWebDAVTestServerData();
}

void WebDAVDeleteJobTest::syncAttribute()
{
    QFETCH(QUrl, url);
    QFETCH(SynqClient::WebDAVServerType, type);

    QNetworkAccessManager nam;

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/WebDAVDeleteJobTest-syncAttribute-" + testDirUid.toString();
    auto remoteFileName = remotePath + "/hello.txt";
    // auto remoteFolderName = remotePath + "/sub";
    QVariant originalSyncAttribute;

    {
        WebDAVCreateDirectoryJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setPath(remotePath);
        QSignalSpy spy(&job, &WebDAVCreateDirectoryJob::finished);
        job.start();
        QVERIFY(spy.wait());
    }

    {
        WebDAVUploadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setData("Hello World!\n");
        job.setRemoteFilename(remoteFileName);
        QSignalSpy spy(&job, &WebDAVUploadFileJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.errorString(), QString());
        QCOMPARE(job.error(), JobError::NoError);
        originalSyncAttribute = job.fileInfo().syncAttribute();

        // TODO: Check why this delay is required.
        // If we do not include a minimal delay here, the following upload will include the SAME
        // etag as the first one - even though the content of the file changed. For "real world" use
        // cases, this probably does not matter (in particular, the sync functionality will usually
        // only try to write each file at most once to the server per sync run). However, it still
        // is annoying.
        QThread::sleep(1);
    }

    {
        WebDAVUploadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setData("Ciao!\n");
        job.setRemoteFilename(remoteFileName);
        QSignalSpy spy(&job, &WebDAVUploadFileJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.errorString(), QString());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        WebDAVDeleteJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setPath(remoteFileName);
        job.setSyncAttribute(originalSyncAttribute);
        QSignalSpy spy(&job, &WebDAVDeleteJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::SyncAttributeMismatch);
    }

    // TODO: Check why conditionally deleting a folder fails.
    // When including the previous etag in the DELETE via Match-If, a delete call will always fail.
    // This is annoying, as it prevents us from implementing a nice and clean delete of folders: We
    // could get a list of the folder's content including its own etag. Then, we create a delete
    // request which only succeeds if the etag matches. However, due to this behaviour, we cannot
    // implement deletion like this.
    /*
        {
            WebDAVCreateDirectoryJob job;
            job.setNetworkAccessManager(&nam);
            job.setUrl(url);
            job.setServerType(type);
            job.setPath(remoteFolderName);
            QSignalSpy spy(&job, &WebDAVCreateDirectoryJob::finished);
            job.start();
            QVERIFY(spy.wait());
            QCOMPARE(job.error(), JobError::NoError);
        }

        {
            WebDAVGetFileInfoJob job;
            job.setNetworkAccessManager(&nam);
            job.setUrl(url);
            job.setServerType(type);
            job.setPath(remoteFolderName);
            QSignalSpy spy(&job, &WebDAVCreateDirectoryJob::finished);
            job.start();
            QVERIFY(spy.wait());
            QCOMPARE(job.error(), JobError::NoError);
            originalSyncAttribute = job.fileInfo().syncAttribute();
            QThread::sleep(1);
        }

        {
            WebDAVDeleteJob job;
            job.setNetworkAccessManager(&nam);
            job.setUrl(url);
            job.setServerType(type);
            job.setPath(remoteFolderName);
            job.setSyncAttribute(originalSyncAttribute);
            QSignalSpy spy(&job, &WebDAVDeleteJob::finished);
            job.start();
            QVERIFY(spy.wait());
            QCOMPARE(job.error(), JobError::NoError);
        }
        */
}

void WebDAVDeleteJobTest::syncAttribute_data()
{
    SynqClient::UnitTest::setupWebDAVTestServerData();
}

void WebDAVDeleteJobTest::cleanupTestCase() {}

QTEST_MAIN(WebDAVDeleteJobTest)

#include "tst_webdavdeletejob.moc"
