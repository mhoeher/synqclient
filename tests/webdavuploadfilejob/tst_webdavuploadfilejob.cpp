#include <QBuffer>
#include <QDir>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUuid>
#include <QtTest>

// add necessary includes here
#include "../shared/utils.h"
#include "WebDAVCreateDirectoryJob"
#include "WebDAVGetFileInfoJob"
#include "WebDAVUploadFileJob"

using SynqClient::JobError;
using SynqClient::WebDAVCreateDirectoryJob;
using SynqClient::WebDAVGetFileInfoJob;
using SynqClient::WebDAVUploadFileJob;

class WebDAVUploadFileJobTest : public QObject
{
    Q_OBJECT

public:
    WebDAVUploadFileJobTest();
    ~WebDAVUploadFileJobTest();

private slots:
    void initTestCase();
    void uploadLocalFile();
    void uploadLocalFile_data();
    void uploadDevice();
    void uploadDevice_data();
    void uploadData();
    void uploadData_data();
    void uploadSyncAttribute();
    void uploadSyncAttribute_data();
    void cleanupTestCase();
};

WebDAVUploadFileJobTest::WebDAVUploadFileJobTest() {}

WebDAVUploadFileJobTest::~WebDAVUploadFileJobTest() {}

void WebDAVUploadFileJobTest::initTestCase() {}

void WebDAVUploadFileJobTest::uploadLocalFile()
{
    if (!SynqClient::UnitTest::hasWebDAVServersFromEnv()) {
        QSKIP("No WebDAV servers configured - skipping test");
    }

    QFETCH(QUrl, url);
    QFETCH(SynqClient::WebDAVServerType, type);

    QNetworkAccessManager nam;

    QTemporaryDir tmpDir;
    QDir dir(tmpDir.path());
    auto localFileName = dir.absoluteFilePath("test.txt");

    QFile localFile(localFileName);
    QVERIFY(localFile.open(QIODevice::WriteOnly));
    localFile.write("Hello World!\n");
    localFile.close();

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/WebDAVUploadFileJobTest-uploadLocalFile-" + testDirUid.toString();
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
        job.setLocalFilename(localFileName);
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
}

void WebDAVUploadFileJobTest::uploadLocalFile_data()
{
    SynqClient::UnitTest::setupWebDAVTestServerData();
}

void WebDAVUploadFileJobTest::uploadDevice()
{
    if (!SynqClient::UnitTest::hasWebDAVServersFromEnv()) {
        QSKIP("No WebDAV servers configured - skipping test");
    }

    QFETCH(QUrl, url);
    QFETCH(SynqClient::WebDAVServerType, type);

    QNetworkAccessManager nam;

    QByteArray data = "Hello World!\n";
    auto buffer = new QBuffer(&data);
    QVERIFY(buffer->open(QIODevice::ReadOnly));

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/WebDAVUploadFileJobTest-uploadDevice-" + testDirUid.toString();
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
        job.setInput(buffer);
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
}

void WebDAVUploadFileJobTest::uploadDevice_data()
{
    SynqClient::UnitTest::setupWebDAVTestServerData();
}

void WebDAVUploadFileJobTest::uploadData()
{
    if (!SynqClient::UnitTest::hasWebDAVServersFromEnv()) {
        QSKIP("No WebDAV servers configured - skipping test");
    }

    QFETCH(QUrl, url);
    QFETCH(SynqClient::WebDAVServerType, type);

    QNetworkAccessManager nam;

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/WebDAVUploadFileJobTest-uploadData-" + testDirUid.toString();
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
}

void WebDAVUploadFileJobTest::uploadData_data()
{
    SynqClient::UnitTest::setupWebDAVTestServerData();
}

void WebDAVUploadFileJobTest::uploadSyncAttribute()
{
    if (!SynqClient::UnitTest::hasWebDAVServersFromEnv()) {
        QSKIP("No WebDAV servers configured - skipping test");
    }

    QFETCH(QUrl, url);
    QFETCH(SynqClient::WebDAVServerType, type);

    QNetworkAccessManager nam;

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/WebDAVUploadFileJobTest-uploadSyncAttribute-" + testDirUid.toString();
    auto remoteFileName = remotePath + "/hello.txt";

    QVariant originalEtag;

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
        originalEtag = job.fileInfo().syncAttribute();

        // TODO: Check why this delay is required.
        // As noted already somewhere else: We need to add a small delay, otherwise, the etag we get
        // back is not updated.
        QThread::sleep(1);
    }

    {
        // Override the file without checking for etag (i.e. update by other client).
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
        // "First" client uploads again, but does not know about the
        // update of the other one, i.e. update should fail:
        WebDAVUploadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setData("Hello again!\n");
        job.setRemoteFilename(remoteFileName);
        job.setSyncAttribute(originalEtag);
        QSignalSpy spy(&job, &WebDAVUploadFileJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::SyncAttributeMismatch);
    }
}

void WebDAVUploadFileJobTest::uploadSyncAttribute_data()
{
    SynqClient::UnitTest::setupWebDAVTestServerData();
}

void WebDAVUploadFileJobTest::cleanupTestCase() {}

QTEST_MAIN(WebDAVUploadFileJobTest)

#include "tst_webdavuploadfilejob.moc"
