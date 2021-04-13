#include <QtTest>

// add necessary includes here
#include "SynqClient/DropboxCreateDirectoryJob"
#include "SynqClient/DropboxDeleteJob"
#include "SynqClient/DropboxDownloadFileJob"
#include "SynqClient/DropboxGetFileInfoJob"
#include "SynqClient/DropboxJobFactory"
#include "SynqClient/DropboxListFilesJob"
#include "SynqClient/DropboxUploadFileJob"

using SynqClient::DropboxCreateDirectoryJob;
using SynqClient::DropboxDeleteJob;
using SynqClient::DropboxDownloadFileJob;
using SynqClient::DropboxGetFileInfoJob;
using SynqClient::DropboxJobFactory;
using SynqClient::DropboxListFilesJob;
using SynqClient::DropboxUploadFileJob;

class DropboxJobFactoryTest : public QObject
{
    Q_OBJECT

public:
    DropboxJobFactoryTest();
    ~DropboxJobFactoryTest();

private slots:
    void initTestCase();
    void createJobs();
    void cleanupTestCase();
};

DropboxJobFactoryTest::DropboxJobFactoryTest() {}

DropboxJobFactoryTest::~DropboxJobFactoryTest() {}

void DropboxJobFactoryTest::initTestCase() {}

void DropboxJobFactoryTest::createJobs()
{
    DropboxJobFactory factory;
    QNetworkAccessManager nam;
    factory.setNetworkAccessManager(&nam);
    factory.setUserAgent("Unit Test");
    factory.setToken("12345");

    {
        auto job = factory.createDirectory(&factory);
        auto davJob = qobject_cast<DropboxCreateDirectoryJob*>(job);
        QVERIFY(davJob != nullptr);
        QCOMPARE(davJob->networkAccessManager(), &nam);
        QCOMPARE(davJob->userAgent(), "Unit Test");
        QCOMPARE(davJob->token(), "12345");
    }

    {
        auto job = factory.deleteResource(&factory);
        auto davJob = qobject_cast<DropboxDeleteJob*>(job);
        QVERIFY(davJob != nullptr);
        QCOMPARE(davJob->networkAccessManager(), &nam);
        QCOMPARE(davJob->userAgent(), "Unit Test");
        QCOMPARE(davJob->token(), "12345");
    }

    {
        auto job = factory.downloadFile(&factory);
        auto davJob = qobject_cast<DropboxDownloadFileJob*>(job);
        QVERIFY(davJob != nullptr);
        QCOMPARE(davJob->networkAccessManager(), &nam);
        QCOMPARE(davJob->userAgent(), "Unit Test");
        QCOMPARE(davJob->token(), "12345");
    }

    {
        auto job = factory.getFileInfo(&factory);
        auto davJob = qobject_cast<DropboxGetFileInfoJob*>(job);
        QVERIFY(davJob != nullptr);
        QCOMPARE(davJob->networkAccessManager(), &nam);
        QCOMPARE(davJob->userAgent(), "Unit Test");
        QCOMPARE(davJob->token(), "12345");
    }

    {
        auto job = factory.listFiles(&factory);
        auto davJob = qobject_cast<DropboxListFilesJob*>(job);
        QVERIFY(davJob != nullptr);
        QCOMPARE(davJob->networkAccessManager(), &nam);
        QCOMPARE(davJob->userAgent(), "Unit Test");
        QCOMPARE(davJob->token(), "12345");
    }

    {
        auto job = factory.uploadFile(&factory);
        auto davJob = qobject_cast<DropboxUploadFileJob*>(job);
        QVERIFY(davJob != nullptr);
        QCOMPARE(davJob->networkAccessManager(), &nam);
        QCOMPARE(davJob->userAgent(), "Unit Test");
        QCOMPARE(davJob->token(), "12345");
    }
}

void DropboxJobFactoryTest::cleanupTestCase() {}

QTEST_MAIN(DropboxJobFactoryTest)

#include "tst_dropboxjobfactory.moc"
