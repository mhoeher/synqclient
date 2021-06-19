#include <QtTest>

// add necessary includes here
#include "../shared/utils.h"
#include "SynqClient/DropboxCreateDirectoryJob"
#include "SynqClient/DropboxDeleteJob"
#include "SynqClient/DropboxGetFileInfoJob"
#include "SynqClient/DropboxListFilesJob"
#include "SynqClient/DropboxUploadFileJob"

using SynqClient::DropboxCreateDirectoryJob;
using SynqClient::DropboxDeleteJob;
using SynqClient::DropboxGetFileInfoJob;
using SynqClient::DropboxListFilesJob;
using SynqClient::DropboxUploadFileJob;
using SynqClient::JobError;

class DropboxDeleteJobTest : public QObject
{
    Q_OBJECT

public:
    DropboxDeleteJobTest();
    ~DropboxDeleteJobTest();

private slots:
    void initTestCase();
    void deleteSingleFile();
    void deleteEmptyFolder();
    void deleteFolderRecursively();
    void syncAttribute();
    void cleanupTestCase();
};

DropboxDeleteJobTest::DropboxDeleteJobTest() {}

DropboxDeleteJobTest::~DropboxDeleteJobTest() {}

void DropboxDeleteJobTest::initTestCase() {}

void DropboxDeleteJobTest::deleteSingleFile()
{
    if (!SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        QSKIP("No Dropbox token configured - skipping test");
    }

    QNetworkAccessManager nam;
    nam.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/DropboxDeleteJobTest-deleteSingleFile-" + testDirUid.toString();
    auto remoteFileName = remotePath + "/hello.txt";

    {
        DropboxCreateDirectoryJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remotePath);
        QSignalSpy spy(&job, &DropboxCreateDirectoryJob::finished);
        job.start();
        QVERIFY(spy.wait());
    }

    {
        DropboxUploadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setData("Hello World!\n");
        job.setRemoteFilename(remoteFileName);
        QSignalSpy spy(&job, &DropboxUploadFileJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.errorString(), QString());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        DropboxGetFileInfoJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remoteFileName);
        QSignalSpy spy(&job, &DropboxGetFileInfoJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        DropboxDeleteJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remoteFileName);
        QSignalSpy spy(&job, &DropboxDeleteJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        DropboxGetFileInfoJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remoteFileName);
        QSignalSpy spy(&job, &DropboxGetFileInfoJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::ResourceNotFound);
    }
}

void DropboxDeleteJobTest::deleteEmptyFolder()
{
    if (!SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        QSKIP("No Dropbox token configured - skipping test");
    }

    QNetworkAccessManager nam;
    nam.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/DropboxDeleteJobTest-deleteEmptyFolder-" + testDirUid.toString();
    auto remoteFolderName = remotePath + "/hello";

    {
        DropboxCreateDirectoryJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remotePath);
        QSignalSpy spy(&job, &DropboxCreateDirectoryJob::finished);
        job.start();
        QVERIFY(spy.wait());
    }

    {
        DropboxCreateDirectoryJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remoteFolderName);
        QSignalSpy spy(&job, &DropboxCreateDirectoryJob::finished);
        job.start();
        QVERIFY(spy.wait());
    }

    {
        DropboxGetFileInfoJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remoteFolderName);
        QSignalSpy spy(&job, &DropboxGetFileInfoJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        DropboxDeleteJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remoteFolderName);
        QSignalSpy spy(&job, &DropboxDeleteJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        DropboxGetFileInfoJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remoteFolderName);
        QSignalSpy spy(&job, &DropboxGetFileInfoJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::ResourceNotFound);
    }
}

void DropboxDeleteJobTest::deleteFolderRecursively()
{
    if (!SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        QSKIP("No Dropbox token configured - skipping test");
    }

    QNetworkAccessManager nam;
    nam.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/DropboxDeleteJobTest-deleteFolderRecursively-" + testDirUid.toString();
    auto remoteSubFolder = remotePath + "/sub-folder";
    auto remoteFileName = remoteSubFolder + "/hello.txt";
    auto remoteFolderName = remoteSubFolder + "/dir";
    auto remoteFileInFolderName = remoteFolderName + "/hello2.txt";

    for (const auto& path : { remotePath, remoteSubFolder, remoteFolderName }) {
        DropboxCreateDirectoryJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(path);
        QSignalSpy spy(&job, &DropboxCreateDirectoryJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.errorString(), QString());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        DropboxUploadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setData("Hello World!\n");
        job.setRemoteFilename(remoteFileName);
        QSignalSpy spy(&job, &DropboxUploadFileJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.errorString(), QString());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        DropboxGetFileInfoJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remoteFileName);
        QSignalSpy spy(&job, &DropboxGetFileInfoJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        DropboxUploadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setData("Still there?\n");
        job.setRemoteFilename(remoteFileInFolderName);
        QSignalSpy spy(&job, &DropboxUploadFileJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.errorString(), QString());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        DropboxGetFileInfoJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remoteFileInFolderName);
        QSignalSpy spy(&job, &DropboxGetFileInfoJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        DropboxDeleteJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remoteSubFolder);
        QSignalSpy spy(&job, &DropboxDeleteJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        DropboxGetFileInfoJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remoteSubFolder);
        QSignalSpy spy(&job, &DropboxGetFileInfoJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::ResourceNotFound);
    }

    {
        DropboxListFilesJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remotePath);
        QSignalSpy spy(&job, &DropboxListFilesJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.entries().length(), 0);
    }
}

void DropboxDeleteJobTest::syncAttribute()
{
    if (!SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        QSKIP("No Dropbox token configured - skipping test");
    }

    QNetworkAccessManager nam;
    nam.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/DropboxDeleteJobTest-syncAttribute-" + testDirUid.toString();
    auto remoteFileName = remotePath + "/hello.txt";
    // auto remoteFolderName = remotePath + "/sub";
    QVariant originalSyncAttribute;

    {
        DropboxCreateDirectoryJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remotePath);
        QSignalSpy spy(&job, &DropboxCreateDirectoryJob::finished);
        job.start();
        QVERIFY(spy.wait());
    }

    {
        DropboxUploadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setData("Hello World!\n");
        job.setRemoteFilename(remoteFileName);
        QSignalSpy spy(&job, &DropboxUploadFileJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.errorString(), QString());
        QCOMPARE(job.error(), JobError::NoError);
        originalSyncAttribute = job.fileInfo().syncAttribute();
    }

    {
        DropboxUploadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setData("Ciao!\n");
        job.setRemoteFilename(remoteFileName);
        QSignalSpy spy(&job, &DropboxUploadFileJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.errorString(), QString());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        DropboxDeleteJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remoteFileName);
        job.setSyncAttribute(originalSyncAttribute);
        QSignalSpy spy(&job, &DropboxDeleteJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::SyncAttributeMismatch);
    }

    // Folders don't have a "rev" property, hence, we cannot delete them conditionally.
}

void DropboxDeleteJobTest::cleanupTestCase() {}

QTEST_MAIN(DropboxDeleteJobTest)

#include "tst_dropboxdeletejob.moc"
