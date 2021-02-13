#include <QSignalSpy>
#include <QUuid>
#include <QtTest>

// add necessary includes here
#include "WebDAVCreateDirectoryJob"
#include "WebDAVGetFileInfoJob"
#include "../shared/utils.h"

class WebDAVCreateDirectoryJobTest : public QObject
{
    Q_OBJECT

public:
    WebDAVCreateDirectoryJobTest();
    ~WebDAVCreateDirectoryJobTest();

private slots:
    void initTestCase();
    void mkdir();
    void mkdir_data();
    void mkdirInvalidPath();
    void mkdirInvalidPath_data();
    void mkdirExisting();
    void mkdirExisting_data();
    void cleanupTestCase();
};

WebDAVCreateDirectoryJobTest::WebDAVCreateDirectoryJobTest() {}

WebDAVCreateDirectoryJobTest::~WebDAVCreateDirectoryJobTest() {}

void WebDAVCreateDirectoryJobTest::initTestCase() {}

void WebDAVCreateDirectoryJobTest::mkdir()
{
    if (!SynqClient::UnitTest::hasWebDAVServersFromEnv()) {
        QSKIP("No WebDAV servers configured - skipping test");
    }

    QFETCH(QUrl, url);
    QFETCH(SynqClient::WebDAVServerType, type);

    QNetworkAccessManager nam;
    auto uuid = QUuid::createUuid();
    auto path = "/WebDAVCreateDirectoryJobTest-mkdir-" + uuid.toString();
    SynqClient::WebDAVCreateDirectoryJob mkdirJob;
    mkdirJob.setNetworkAccessManager(&nam);
    mkdirJob.setServerType(type);
    mkdirJob.setUrl(url);
    mkdirJob.setPath(path);
    mkdirJob.start();
    QSignalSpy mkdirSpy(&mkdirJob, &SynqClient::WebDAVCreateDirectoryJob::finished);
    QVERIFY(mkdirSpy.wait());
    QCOMPARE(mkdirJob.error(), SynqClient::JobError::NoError);

    SynqClient::WebDAVGetFileInfoJob getFileInfoJob;
    getFileInfoJob.setNetworkAccessManager(&nam);
    getFileInfoJob.setServerType(type);
    getFileInfoJob.setUrl(url);
    getFileInfoJob.setPath(path);
    getFileInfoJob.start();
    QSignalSpy getFileInfoSpy(&getFileInfoJob, &SynqClient::AbstractJob::finished);
    QVERIFY(getFileInfoSpy.wait());
    QCOMPARE(getFileInfoJob.error(), SynqClient::JobError::NoError);
    auto fileInfo = getFileInfoJob.fileInfo();
    QVERIFY(fileInfo.isValid());
    QVERIFY(fileInfo.isDirectory());
    QCOMPARE(fileInfo.name(), ".");
}

void WebDAVCreateDirectoryJobTest::mkdir_data()
{
    SynqClient::UnitTest::setupWebDAVTestServerData();
}

void WebDAVCreateDirectoryJobTest::mkdirInvalidPath()
{
    if (!SynqClient::UnitTest::hasWebDAVServersFromEnv()) {
        QSKIP("No WebDAV servers configured - skipping test");
    }

    QFETCH(QUrl, url);
    QFETCH(SynqClient::WebDAVServerType, type);

    QNetworkAccessManager nam;
    auto uuid = QUuid::createUuid();
    auto path = "/WebDAVCreateDirectoryJobTest-mkdirInvalidPath-" + uuid.toString();
    SynqClient::WebDAVCreateDirectoryJob mkdirJob;
    mkdirJob.setNetworkAccessManager(&nam);
    mkdirJob.setServerType(type);
    mkdirJob.setUrl(url);
    mkdirJob.setPath(path + "/sub-folder");
    mkdirJob.start();
    QSignalSpy mkdirSpy(&mkdirJob, &SynqClient::WebDAVCreateDirectoryJob::finished);
    QVERIFY(mkdirSpy.wait());
    QCOMPARE(mkdirJob.error(), SynqClient::JobError::ServerContentConflict);

    SynqClient::WebDAVGetFileInfoJob getFileInfoJob;
    getFileInfoJob.setNetworkAccessManager(&nam);
    getFileInfoJob.setServerType(type);
    getFileInfoJob.setUrl(url);
    getFileInfoJob.setPath(path);
    getFileInfoJob.start();
    QSignalSpy getFileInfoSpy(&getFileInfoJob, &SynqClient::AbstractJob::finished);
    QVERIFY(getFileInfoSpy.wait());
    QCOMPARE(getFileInfoJob.error(), SynqClient::JobError::ResourceNotFound);
    auto fileInfo = getFileInfoJob.fileInfo();
    QVERIFY(!fileInfo.isValid());
}

void WebDAVCreateDirectoryJobTest::mkdirInvalidPath_data()
{
    SynqClient::UnitTest::setupWebDAVTestServerData();
}

void WebDAVCreateDirectoryJobTest::mkdirExisting()
{
    if (!SynqClient::UnitTest::hasWebDAVServersFromEnv()) {
        QSKIP("No WebDAV servers configured - skipping test");
    }

    QFETCH(QUrl, url);
    QFETCH(SynqClient::WebDAVServerType, type);

    QNetworkAccessManager nam;
    auto uuid = QUuid::createUuid();
    auto path = "/WebDAVCreateDirectoryJobTest-mkdir-" + uuid.toString();

    {
        SynqClient::WebDAVCreateDirectoryJob mkdirJob;
        mkdirJob.setNetworkAccessManager(&nam);
        mkdirJob.setServerType(type);
        mkdirJob.setUrl(url);
        mkdirJob.setPath(path);
        mkdirJob.start();
        QSignalSpy mkdirSpy(&mkdirJob, &SynqClient::WebDAVCreateDirectoryJob::finished);
        QVERIFY(mkdirSpy.wait());
        QCOMPARE(mkdirJob.error(), SynqClient::JobError::NoError);
    }

    {
        SynqClient::WebDAVGetFileInfoJob getFileInfoJob;
        getFileInfoJob.setNetworkAccessManager(&nam);
        getFileInfoJob.setServerType(type);
        getFileInfoJob.setUrl(url);
        getFileInfoJob.setPath(path);
        getFileInfoJob.start();
        QSignalSpy getFileInfoSpy(&getFileInfoJob, &SynqClient::AbstractJob::finished);
        QVERIFY(getFileInfoSpy.wait());
        QCOMPARE(getFileInfoJob.error(), SynqClient::JobError::NoError);
        auto fileInfo = getFileInfoJob.fileInfo();
        QVERIFY(fileInfo.isValid());
        QVERIFY(fileInfo.isDirectory());
        QCOMPARE(fileInfo.name(), ".");
    }

    {
        SynqClient::WebDAVCreateDirectoryJob mkdirJob;
        mkdirJob.setNetworkAccessManager(&nam);
        mkdirJob.setServerType(type);
        mkdirJob.setUrl(url);
        mkdirJob.setPath(path);
        mkdirJob.start();
        QSignalSpy mkdirSpy(&mkdirJob, &SynqClient::WebDAVCreateDirectoryJob::finished);
        QVERIFY(mkdirSpy.wait());
        QCOMPARE(mkdirJob.error(), SynqClient::JobError::FolderExists);
    }
}

void WebDAVCreateDirectoryJobTest::mkdirExisting_data()
{
    SynqClient::UnitTest::setupWebDAVTestServerData();
}

void WebDAVCreateDirectoryJobTest::cleanupTestCase() {}

QTEST_MAIN(WebDAVCreateDirectoryJobTest)

#include "tst_webdavcreatedirectoryjob.moc"
