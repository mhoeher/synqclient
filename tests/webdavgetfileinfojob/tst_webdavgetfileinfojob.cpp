#include <QNetworkAccessManager>
#include <QSignalSpy>
#include <QUuid>
#include <QtTest>

// add necessary includes here
#include "WebDAVGetFileInfoJob"
#include "../shared/utils.h"

class WebDAVGetFileInfoJobTest : public QObject
{
    Q_OBJECT

public:
    WebDAVGetFileInfoJobTest();
    ~WebDAVGetFileInfoJobTest();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void getRootItemFileInfo();
    void getRootItemFileInfo_data();
    void getFileInfoForNonExistingFile();
    void getFileInfoForNonExistingFile_data();
};

WebDAVGetFileInfoJobTest::WebDAVGetFileInfoJobTest() {}

WebDAVGetFileInfoJobTest::~WebDAVGetFileInfoJobTest() {}

void WebDAVGetFileInfoJobTest::initTestCase() {}

void WebDAVGetFileInfoJobTest::cleanupTestCase() {}

void WebDAVGetFileInfoJobTest::getRootItemFileInfo()
{
    QFETCH(QUrl, url);
    QFETCH(SynqClient::WebDAVServerType, type);

    QNetworkAccessManager nam;
    SynqClient::WebDAVGetFileInfoJob job;
    job.setNetworkAccessManager(&nam);
    job.setServerType(type);
    job.setUrl(url);
    job.start();
    QSignalSpy spy(&job, &SynqClient::AbstractJob::finished);
    QVERIFY(spy.wait());
    QCOMPARE(job.error(), SynqClient::JobError::NoError);
    auto fileInfo = job.fileInfo();
    QCOMPARE(fileInfo[SynqClient::ItemProperty::Name], ".");
}

void WebDAVGetFileInfoJobTest::getRootItemFileInfo_data()
{
    SynqClient::UnitTest::setupWebDAVTestServerData();
}

void WebDAVGetFileInfoJobTest::getFileInfoForNonExistingFile()
{
    QFETCH(QUrl, url);
    QFETCH(SynqClient::WebDAVServerType, type);

    auto uid = QUuid::createUuid();
    auto path = "/WebDAVGetFileInfoJobTest-should-definitely-not-exist-" + uid.toString();
    QNetworkAccessManager nam;
    SynqClient::WebDAVGetFileInfoJob job;
    job.setNetworkAccessManager(&nam);
    job.setServerType(type);
    job.setUrl(url);
    job.setPath(path);
    job.start();
    QSignalSpy spy(&job, &SynqClient::AbstractJob::finished);
    QVERIFY(spy.wait());
    QCOMPARE(job.error(), SynqClient::JobError::ResourceNotFound);
    auto fileInfo = job.fileInfo();
    QVERIFY(fileInfo.isEmpty());
}

void WebDAVGetFileInfoJobTest::getFileInfoForNonExistingFile_data()
{
    SynqClient::UnitTest::setupWebDAVTestServerData();
}

QTEST_MAIN(WebDAVGetFileInfoJobTest)

#include "tst_webdavgetfileinfojob.moc"
