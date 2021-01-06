#include <QtTest>
#include <QNetworkAccessManager>

// add necessary includes here
#include "WebDAVCreateDirectoryJob"
#include "WebDAVDeleteJob"
#include "WebDAVDownloadFileJob"
#include "WebDAVGetFileInfoJob"
#include "WebDAVJobFactory"
#include "WebDAVListFilesJob"
#include "WebDAVUploadFileJob"

using SynqClient::WebDAVCreateDirectoryJob;
using SynqClient::WebDAVDeleteJob;
using SynqClient::WebDAVDownloadFileJob;
using SynqClient::WebDAVGetFileInfoJob;
using SynqClient::WebDAVJobFactory;
using SynqClient::WebDAVListFilesJob;
using SynqClient::WebDAVServerType;
using SynqClient::WebDAVUploadFileJob;

class WebDAVJobFactoryTest : public QObject
{
    Q_OBJECT

public:
    WebDAVJobFactoryTest();
    ~WebDAVJobFactoryTest();

private slots:
    void initTestCase();
    void createJobs();
    void cleanupTestCase();
};

WebDAVJobFactoryTest::WebDAVJobFactoryTest() {}

WebDAVJobFactoryTest::~WebDAVJobFactoryTest() {}

void WebDAVJobFactoryTest::initTestCase() {}

void WebDAVJobFactoryTest::createJobs()
{
    WebDAVJobFactory factory;
    QNetworkAccessManager nam;
    factory.setNetworkAccessManager(&nam);
    factory.setServerType(WebDAVServerType::NextCloud);
    factory.setUrl(QUrl("https://example.com"));
    factory.setUserAgent("Unit Test");

    {
        auto job = factory.createDirectory(&factory);
        auto davJob = qobject_cast<WebDAVCreateDirectoryJob*>(job);
        QVERIFY(davJob != nullptr);
        QCOMPARE(davJob->networkAccessManager(), &nam);
        QCOMPARE(davJob->serverType(), WebDAVServerType::NextCloud);
        QCOMPARE(davJob->url(), QUrl("https://example.com"));
        QCOMPARE(davJob->userAgent(), "Unit Test");
    }

    {
        auto job = factory.deleteResource(&factory);
        auto davJob = qobject_cast<WebDAVDeleteJob*>(job);
        QVERIFY(davJob != nullptr);
        QCOMPARE(davJob->networkAccessManager(), &nam);
        QCOMPARE(davJob->serverType(), WebDAVServerType::NextCloud);
        QCOMPARE(davJob->url(), QUrl("https://example.com"));
        QCOMPARE(davJob->userAgent(), "Unit Test");
    }

    {
        auto job = factory.downloadFile(&factory);
        auto davJob = qobject_cast<WebDAVDownloadFileJob*>(job);
        QVERIFY(davJob != nullptr);
        QCOMPARE(davJob->networkAccessManager(), &nam);
        QCOMPARE(davJob->serverType(), WebDAVServerType::NextCloud);
        QCOMPARE(davJob->url(), QUrl("https://example.com"));
        QCOMPARE(davJob->userAgent(), "Unit Test");
    }

    {
        auto job = factory.getFileInfo(&factory);
        auto davJob = qobject_cast<WebDAVGetFileInfoJob*>(job);
        QVERIFY(davJob != nullptr);
        QCOMPARE(davJob->networkAccessManager(), &nam);
        QCOMPARE(davJob->serverType(), WebDAVServerType::NextCloud);
        QCOMPARE(davJob->url(), QUrl("https://example.com"));
        QCOMPARE(davJob->userAgent(), "Unit Test");
    }

    {
        auto job = factory.listFiles(&factory);
        auto davJob = qobject_cast<WebDAVListFilesJob*>(job);
        QVERIFY(davJob != nullptr);
        QCOMPARE(davJob->networkAccessManager(), &nam);
        QCOMPARE(davJob->serverType(), WebDAVServerType::NextCloud);
        QCOMPARE(davJob->url(), QUrl("https://example.com"));
        QCOMPARE(davJob->userAgent(), "Unit Test");
    }

    {
        auto job = factory.uploadFile(&factory);
        auto davJob = qobject_cast<WebDAVUploadFileJob*>(job);
        QVERIFY(davJob != nullptr);
        QCOMPARE(davJob->networkAccessManager(), &nam);
        QCOMPARE(davJob->serverType(), WebDAVServerType::NextCloud);
        QCOMPARE(davJob->url(), QUrl("https://example.com"));
        QCOMPARE(davJob->userAgent(), "Unit Test");
    }
}

void WebDAVJobFactoryTest::cleanupTestCase() {}

QTEST_MAIN(WebDAVJobFactoryTest)

#include "tst_webdavjobfactory.moc"