#include <QBuffer>
#include <QDir>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUuid>
#include <QtTest>

// add necessary includes here
#include "../shared/utils.h"
#include "SynqClient/WebDAVCreateDirectoryJob"
#include "SynqClient/WebDAVDownloadFileJob"
#include "SynqClient/WebDAVGetFileInfoJob"
#include "SynqClient/WebDAVUploadFileJob"

using SynqClient::JobError;
using SynqClient::WebDAVCreateDirectoryJob;
using SynqClient::WebDAVDownloadFileJob;
using SynqClient::WebDAVGetFileInfoJob;
using SynqClient::WebDAVUploadFileJob;

class WebDAVDownloadFileJobTest : public QObject
{
    Q_OBJECT

public:
    WebDAVDownloadFileJobTest();
    ~WebDAVDownloadFileJobTest();

private slots:
    void initTestCase();
    void downloadLocalFile();
    void downloadLocalFile_data();
    void downloadDevice();
    void downloadDevice_data();
    void downloadData();
    void downloadData_data();
    void cleanupTestCase();
};

WebDAVDownloadFileJobTest::WebDAVDownloadFileJobTest() {}

void WebDAVDownloadFileJobTest::downloadLocalFile()
{
    if (!SynqClient::UnitTest::hasWebDAVServersFromEnv()) {
        QSKIP("No WebDAV servers configured - skipping test");
    }

    QFETCH(QUrl, url);
    QFETCH(SynqClient::WebDAVServerType, type);
    QFETCH(int, flags);

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
    auto remotePath = "/WebDAVDownloadFileJobTest-downloadLocalFile-" + testDirUid.toString();
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
        WebDAVDownloadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setLocalFilename(localFileName2);
        job.setRemoteFilename(remoteFileName);
        QSignalSpy spy(&job, &WebDAVGetFileInfoJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
        QFile f1(localFileName);
        QVERIFY(f1.open(QIODevice::ReadOnly));
        QFile f2(localFileName2);
        QVERIFY(f2.open(QIODevice::ReadOnly));
        QCOMPARE(f2.readAll(), f1.readAll());
        if (!(flags & static_cast<int>(SynqClient::UnitTest::WebDAVServerFlag::NoEtagOnDownload))) {
            QVERIFY(!job.fileInfo().syncAttribute().isEmpty());
        }
    }
}

void WebDAVDownloadFileJobTest::downloadLocalFile_data()
{
    SynqClient::UnitTest::setupWebDAVTestServerData();
}

void WebDAVDownloadFileJobTest::downloadDevice()
{
    if (!SynqClient::UnitTest::hasWebDAVServersFromEnv()) {
        QSKIP("No WebDAV servers configured - skipping test");
    }

    QFETCH(QUrl, url);
    QFETCH(SynqClient::WebDAVServerType, type);
    QFETCH(int, flags);

    QNetworkAccessManager nam;
    nam.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    QByteArray data = "Hello World!\n";
    auto buffer = new QBuffer(&data);
    QVERIFY(buffer->open(QIODevice::ReadOnly));

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/WebDAVDownloadFileJobTest-downloadDevice-" + testDirUid.toString();
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
        WebDAVDownloadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        QByteArray localData;
        QBuffer buffer2(&localData);
        QVERIFY(buffer2.open(QIODevice::ReadWrite));
        job.setOutput(&buffer2);
        job.setRemoteFilename(remoteFileName);
        QSignalSpy spy(&job, &WebDAVGetFileInfoJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
        QCOMPARE(localData, data);
        if (!(flags & static_cast<int>(SynqClient::UnitTest::WebDAVServerFlag::NoEtagOnDownload))) {
            QVERIFY(!job.fileInfo().syncAttribute().isEmpty());
        }
    }
}

void WebDAVDownloadFileJobTest::downloadDevice_data()
{
    SynqClient::UnitTest::setupWebDAVTestServerData();
}

void WebDAVDownloadFileJobTest::downloadData()
{
    if (!SynqClient::UnitTest::hasWebDAVServersFromEnv()) {
        QSKIP("No WebDAV servers configured - skipping test");
    }

    QFETCH(QUrl, url);
    QFETCH(SynqClient::WebDAVServerType, type);
    QFETCH(int, flags);

    QNetworkAccessManager nam;
    nam.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/WebDAVDownloadFileJobTest-downloadData-" + testDirUid.toString();
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
        WebDAVDownloadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setRemoteFilename(remoteFileName);
        QSignalSpy spy(&job, &WebDAVGetFileInfoJob::finished);
        job.start();
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
        QCOMPARE(job.data(), "Hello World!\n");
        if (!(flags & static_cast<int>(SynqClient::UnitTest::WebDAVServerFlag::NoEtagOnDownload))) {
            QVERIFY(!job.fileInfo().syncAttribute().isEmpty());
        }
    }
}

void WebDAVDownloadFileJobTest::downloadData_data()
{
    SynqClient::UnitTest::setupWebDAVTestServerData();
}

WebDAVDownloadFileJobTest::~WebDAVDownloadFileJobTest() {}

void WebDAVDownloadFileJobTest::initTestCase() {}

void WebDAVDownloadFileJobTest::cleanupTestCase() {}

QTEST_MAIN(WebDAVDownloadFileJobTest)

#include "tst_webdavdownloadfilejob.moc"
