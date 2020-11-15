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
    void cleanupTestCase();
};

WebDAVCreateDirectoryJobTest::WebDAVCreateDirectoryJobTest() {}

WebDAVCreateDirectoryJobTest::~WebDAVCreateDirectoryJobTest() {}

void WebDAVCreateDirectoryJobTest::initTestCase() {}

void WebDAVCreateDirectoryJobTest::mkdir()
{
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
    QCOMPARE(fileInfo[SynqClient::ItemProperty::Name], ".");
}

void WebDAVCreateDirectoryJobTest::mkdir_data()
{
    SynqClient::UnitTest::setupWebDAVTestServerData();
}

void WebDAVCreateDirectoryJobTest::mkdirInvalidPath()
{
    QFETCH(QUrl, url);
    QFETCH(SynqClient::WebDAVServerType, type);

    QNetworkAccessManager nam;
    auto uuid = QUuid::createUuid();
    auto path = "/WebDAVCreateDirectoryJobTest-mkdir-" + uuid.toString();
    SynqClient::WebDAVCreateDirectoryJob mkdirJob;
    mkdirJob.setNetworkAccessManager(&nam);
    mkdirJob.setServerType(type);
    mkdirJob.setUrl(url);
    mkdirJob.setPath(path + "/sub-folder");
    mkdirJob.start();
    QSignalSpy mkdirSpy(&mkdirJob, &SynqClient::WebDAVCreateDirectoryJob::finished);
    QVERIFY(mkdirSpy.wait());
    QCOMPARE(mkdirJob.error(), SynqClient::JobError::NetworkRequestFailed);

    SynqClient::WebDAVGetFileInfoJob getFileInfoJob;
    getFileInfoJob.setNetworkAccessManager(&nam);
    getFileInfoJob.setServerType(type);
    getFileInfoJob.setUrl(url);
    getFileInfoJob.setPath(path);
    getFileInfoJob.start();
    QSignalSpy getFileInfoSpy(&getFileInfoJob, &SynqClient::AbstractJob::finished);
    QVERIFY(getFileInfoSpy.wait());
    QCOMPARE(getFileInfoJob.error(), SynqClient::JobError::NetworkRequestFailed);
    auto fileInfo = getFileInfoJob.fileInfo();
    QVERIFY(fileInfo.isEmpty());
}

void WebDAVCreateDirectoryJobTest::mkdirInvalidPath_data()
{
    SynqClient::UnitTest::setupWebDAVTestServerData();
}

void WebDAVCreateDirectoryJobTest::cleanupTestCase() {}

QTEST_MAIN(WebDAVCreateDirectoryJobTest)

#include "tst_webdavcreatedirectoryjob.moc"
