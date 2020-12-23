#include <algorithm>

#include <QtTest>

// add necessary includes here
#include "../shared/utils.h"
#include "WebDAVCreateDirectoryJob"
#include "WebDAVGetFileInfoJob"
#include "WebDAVListFilesJob"
#include "WebDAVUploadFileJob"

using SynqClient::JobError;
using SynqClient::WebDAVCreateDirectoryJob;
using SynqClient::WebDAVGetFileInfoJob;
using SynqClient::WebDAVListFilesJob;
using SynqClient::WebDAVUploadFileJob;

class WebDAVListFilesJobTest : public QObject
{
    Q_OBJECT

public:
    WebDAVListFilesJobTest();
    ~WebDAVListFilesJobTest();

private slots:
    void initTestCase();
    void listFiles();
    void listFiles_data();
    void cleanupTestCase();
};

WebDAVListFilesJobTest::WebDAVListFilesJobTest() {}

WebDAVListFilesJobTest::~WebDAVListFilesJobTest() {}

void WebDAVListFilesJobTest::initTestCase() {}

void WebDAVListFilesJobTest::listFiles()
{
    QFETCH(QUrl, url);
    QFETCH(SynqClient::WebDAVServerType, type);

    QNetworkAccessManager nam;

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/WebDAVListFilesJobTest-listFiles-" + testDirUid.toString();

    {
        WebDAVCreateDirectoryJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setPath(remotePath);
        job.start();
        QSignalSpy spy(&job, &WebDAVCreateDirectoryJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        WebDAVListFilesJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setPath(remotePath);
        job.start();
        QSignalSpy spy(&job, &WebDAVListFilesJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
        QCOMPARE(job.entries().length(), 0);
    }

    {
        WebDAVCreateDirectoryJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setPath(remotePath + "/dir1");
        job.start();
        QSignalSpy spy(&job, &WebDAVCreateDirectoryJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        WebDAVCreateDirectoryJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setPath(remotePath + "/dir2");
        job.start();
        QSignalSpy spy(&job, &WebDAVCreateDirectoryJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        WebDAVUploadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setRemoteFilename(remotePath + "/file1.txt");
        job.setData("Hello World!");
        job.start();
        QSignalSpy spy(&job, &WebDAVUploadFileJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        WebDAVUploadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setRemoteFilename(remotePath + "/file2.txt");
        job.setData("Another file!");
        job.start();
        QSignalSpy spy(&job, &WebDAVUploadFileJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        WebDAVListFilesJob job;
        job.setNetworkAccessManager(&nam);
        job.setUrl(url);
        job.setServerType(type);
        job.setPath(remotePath);
        job.start();
        QSignalSpy spy(&job, &WebDAVListFilesJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
        QCOMPARE(job.entries().length(), 4);

        QStringList expectedNames { "dir1", "dir2", "file1.txt", "file2.txt" };
        QStringList gotNames;
        for (const auto& entry : job.entries()) {
            auto name = entry.name();
            gotNames << name;
            if (name == "dir1" || name == "dir2") {
                QVERIFY(entry.isDirectory());
            }
            if (name == "file1.txt" || name == "file2.txt") {
                QVERIFY(entry.isFile());
            }
        }
        std::sort(gotNames.begin(), gotNames.end());
        QCOMPARE(gotNames, expectedNames);
    }
}

void WebDAVListFilesJobTest::listFiles_data()
{
    SynqClient::UnitTest::setupWebDAVTestServerData();
}

void WebDAVListFilesJobTest::cleanupTestCase() {}

QTEST_MAIN(WebDAVListFilesJobTest)

#include "tst_webdavlistfilesjob.moc"
