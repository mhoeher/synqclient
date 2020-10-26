#include <QtTest>
#include <QSignalSpy>
#include <QNetworkAccessManager>

// add necessary includes here
#include "WebDAVGetFileInfoJob"

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
};

WebDAVGetFileInfoJobTest::WebDAVGetFileInfoJobTest() {}

WebDAVGetFileInfoJobTest::~WebDAVGetFileInfoJobTest() {}

void WebDAVGetFileInfoJobTest::initTestCase() {}

void WebDAVGetFileInfoJobTest::cleanupTestCase() {}

void WebDAVGetFileInfoJobTest::getRootItemFileInfo()
{
    QNetworkAccessManager nam;
    SynqClient::WebDAVGetFileInfoJob job;
    job.setNetworkAccessManager(&nam);
    job.setServerType(SynqClient::WebDAVServerType::NextCloud);
    job.setUrl(QUrl("http://admin:admin@localhost:8080"));
    job.start();
    QSignalSpy spy(&job, &SynqClient::AbstractJob::finished);
    QVERIFY(spy.wait());
    QCOMPARE(job.error(), SynqClient::JobError::NoError);
    auto fileInfo = job.fileInfo();
    QCOMPARE(fileInfo[SynqClient::ItemProperty::Name], ".");
}

QTEST_MAIN(WebDAVGetFileInfoJobTest)

#include "tst_webdavgetfileinfojob.moc"
