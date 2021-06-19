#include <QtTest>

// add necessary includes here
#include "../shared/utils.h"
#include "SynqClient/DropboxCreateDirectoryJob"
#include "SynqClient/DropboxDownloadFileJob"
#include "SynqClient/DropboxGetFileInfoJob"
#include "SynqClient/DropboxUploadFileJob"

using SynqClient::DropboxCreateDirectoryJob;
using SynqClient::DropboxDownloadFileJob;
using SynqClient::DropboxGetFileInfoJob;
using SynqClient::DropboxUploadFileJob;
using SynqClient::JobError;

class DropboxDownloadFileJobTest : public QObject
{
    Q_OBJECT

public:
    DropboxDownloadFileJobTest();
    ~DropboxDownloadFileJobTest();

private slots:
    void initTestCase();
    void downloadLocalFile();
    void downloadDevice();
    void downloadData();
    void cleanupTestCase();
};

DropboxDownloadFileJobTest::DropboxDownloadFileJobTest() {}

DropboxDownloadFileJobTest::~DropboxDownloadFileJobTest() {}

void DropboxDownloadFileJobTest::initTestCase() {}

void DropboxDownloadFileJobTest::downloadLocalFile()
{
    if (!SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        QSKIP("No Dropbox token configured - skipping test");
    }

    QNetworkAccessManager nam;
    nam.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    QTemporaryDir tmpDir;
    QDir dir(tmpDir.path());
    auto localFileName = dir.absoluteFilePath("test.txt");
    auto localFileName2 = dir.absoluteFilePath("test.txt");

    QFile localFile(localFileName);
    QVERIFY(localFile.open(QIODevice::WriteOnly));
    localFile.write("Hello World!\n");
    localFile.close();

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/DropboxDownloadFileJobTest-downloadLocalFile-" + testDirUid.toString();
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
        DropboxDownloadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setLocalFilename(localFileName2);
        job.setRemoteFilename(remoteFileName);
        QSignalSpy spy(&job, &DropboxGetFileInfoJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
        QFile f1(localFileName);
        QVERIFY(f1.open(QIODevice::ReadOnly));
        QFile f2(localFileName2);
        QVERIFY(f2.open(QIODevice::ReadOnly));
        QCOMPARE(f2.readAll(), f1.readAll());
        QVERIFY(!job.fileInfo().syncAttribute().isEmpty());
    }
}

void DropboxDownloadFileJobTest::downloadDevice()
{
    if (!SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        QSKIP("No Dropbox token configured - skipping test");
    }

    QNetworkAccessManager nam;
    nam.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    QByteArray data = "Hello World!\n";
    auto buffer = new QBuffer(&data);
    QVERIFY(buffer->open(QIODevice::ReadOnly));

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/DropboxDownloadFileJobTest-downloadDevice-" + testDirUid.toString();
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
        DropboxDownloadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        QByteArray localData;
        QBuffer buffer2(&localData);
        QVERIFY(buffer2.open(QIODevice::ReadWrite));
        job.setOutput(&buffer2);
        job.setRemoteFilename(remoteFileName);
        QSignalSpy spy(&job, &DropboxGetFileInfoJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
        QCOMPARE(localData, data);
        QVERIFY(!job.fileInfo().syncAttribute().isEmpty());
    }
}

void DropboxDownloadFileJobTest::downloadData()
{
    if (!SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        QSKIP("No Dropbox token configured - skipping test");
    }

    QNetworkAccessManager nam;
    nam.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/DropboxDownloadFileJobTest-downloadData-" + testDirUid.toString();
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
        DropboxDownloadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setRemoteFilename(remoteFileName);
        QSignalSpy spy(&job, &DropboxGetFileInfoJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
        QCOMPARE(job.data(), "Hello World!\n");
        QVERIFY(!job.fileInfo().syncAttribute().isEmpty());
    }
}

void DropboxDownloadFileJobTest::cleanupTestCase() {}

QTEST_MAIN(DropboxDownloadFileJobTest)

#include "tst_dropboxdownloadfilejob.moc"
