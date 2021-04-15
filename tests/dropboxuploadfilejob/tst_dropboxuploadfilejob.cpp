#include <QtTest>

// add necessary includes here
#include "../shared/utils.h"
#include "SynqClient/DropboxCreateDirectoryJob"
#include "SynqClient/DropboxGetFileInfoJob"
#include "SynqClient/DropboxUploadFileJob"

using SynqClient::DropboxCreateDirectoryJob;
using SynqClient::DropboxGetFileInfoJob;
using SynqClient::DropboxUploadFileJob;
using SynqClient::JobError;

class DropboxUploadFileJobTest : public QObject
{
    Q_OBJECT

public:
    DropboxUploadFileJobTest();
    ~DropboxUploadFileJobTest();

private slots:
    void initTestCase();
    void uploadLocalFile();
    void uploadDevice();
    void uploadData();
    void uploadSyncAttribute();
    void cleanupTestCase();
};

DropboxUploadFileJobTest::DropboxUploadFileJobTest() {}

DropboxUploadFileJobTest::~DropboxUploadFileJobTest() {}

void DropboxUploadFileJobTest::initTestCase() {}

void DropboxUploadFileJobTest::uploadLocalFile()
{
    if (!SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        QSKIP("No Dropbox token configured - skipping test");
    }

    QNetworkAccessManager nam;

    QTemporaryDir tmpDir;
    QDir dir(tmpDir.path());
    auto localFileName = dir.absoluteFilePath("test.txt");

    QFile localFile(localFileName);
    QVERIFY(localFile.open(QIODevice::WriteOnly));
    localFile.write("Hello World!\n");
    localFile.close();

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/DropboxUploadFileJobTest-uploadLocalFile-" + testDirUid.toString();
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
        job.setLocalFilename(localFileName);
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
}

void DropboxUploadFileJobTest::uploadDevice()
{
    if (!SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        QSKIP("No Dropbox token configured - skipping test");
    }

    QNetworkAccessManager nam;

    QByteArray data = "Hello World!\n";
    auto buffer = new QBuffer(&data);
    QVERIFY(buffer->open(QIODevice::ReadOnly));

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/DropboxUploadFileJobTest-uploadDevice-" + testDirUid.toString();
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
        job.setInput(buffer);
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
}

void DropboxUploadFileJobTest::uploadData()
{
    if (!SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        QSKIP("No Dropbox token configured - skipping test");
    }

    QNetworkAccessManager nam;

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/DropboxUploadFileJobTest-uploadData-" + testDirUid.toString();
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
}

void DropboxUploadFileJobTest::uploadSyncAttribute()
{
    if (!SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        QSKIP("No Dropbox token configured - skipping test");
    }

    QNetworkAccessManager nam;

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/DropboxUploadFileJobTest-uploadSyncAttribute-" + testDirUid.toString();
    auto remoteFileName = remotePath + "/hello.txt";

    QVariant originalEtag;

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
        originalEtag = job.fileInfo().syncAttribute();
    }

    {
        // Override the file without checking for etag (i.e. update by other client).
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
        // "First" client uploads again, but does not know about the
        // update of the other one, i.e. update should fail:
        DropboxUploadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setData("Hello again!\n");
        job.setRemoteFilename(remoteFileName);
        job.setSyncAttribute(originalEtag);
        QSignalSpy spy(&job, &DropboxUploadFileJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::SyncAttributeMismatch);
    }
}

void DropboxUploadFileJobTest::cleanupTestCase() {}

QTEST_MAIN(DropboxUploadFileJobTest)

#include "tst_dropboxuploadfilejob.moc"
