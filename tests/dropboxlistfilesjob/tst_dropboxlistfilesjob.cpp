#include <QtTest>

// add necessary includes here
#include "../shared/utils.h"
#include "SynqClient/DropboxCreateDirectoryJob"
#include "SynqClient/DropboxGetFileInfoJob"
#include "SynqClient/DropboxListFilesJob"
#include "SynqClient/DropboxUploadFileJob"

using SynqClient::DropboxCreateDirectoryJob;
using SynqClient::DropboxGetFileInfoJob;
using SynqClient::DropboxListFilesJob;
using SynqClient::DropboxUploadFileJob;
using SynqClient::JobError;

class DropboxListFilesJobTest : public QObject
{
    Q_OBJECT

public:
    DropboxListFilesJobTest();
    ~DropboxListFilesJobTest();

private slots:
    void initTestCase();
    void listFiles();
    void cleanupTestCase();
};

DropboxListFilesJobTest::DropboxListFilesJobTest() {}

DropboxListFilesJobTest::~DropboxListFilesJobTest() {}

void DropboxListFilesJobTest::initTestCase() {}

void DropboxListFilesJobTest::listFiles()
{
    if (!SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        QSKIP("No Dropbox token configured - skipping test");
    }

    QNetworkAccessManager nam;
    nam.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/DropboxListFilesJobTest-listFiles-" + testDirUid.toString();

    {
        DropboxCreateDirectoryJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remotePath);
        job.start();
        QSignalSpy spy(&job, &DropboxCreateDirectoryJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        DropboxListFilesJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remotePath);
        job.start();
        QSignalSpy spy(&job, &DropboxListFilesJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
        QCOMPARE(job.entries().length(), 0);
        QVERIFY(job.folder().isValid());
        QCOMPARE(job.folder().name(), ".");
    }

    {
        DropboxCreateDirectoryJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remotePath + "/dir1");
        job.start();
        QSignalSpy spy(&job, &DropboxCreateDirectoryJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        DropboxCreateDirectoryJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remotePath + "/dir2");
        job.start();
        QSignalSpy spy(&job, &DropboxCreateDirectoryJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        DropboxUploadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setRemoteFilename(remotePath + "/file1.txt");
        job.setData("Hello World!");
        job.start();
        QSignalSpy spy(&job, &DropboxUploadFileJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        DropboxUploadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setRemoteFilename(remotePath + "/file2.txt");
        job.setData("Another file!");
        job.start();
        QSignalSpy spy(&job, &DropboxUploadFileJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    {
        DropboxListFilesJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remotePath);
        job.start();
        QSignalSpy spy(&job, &DropboxListFilesJob::finished);
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
        QVERIFY(job.folder().isValid());
        QCOMPARE(job.folder().name(), ".");
        QVERIFY(job.folder().isDirectory());
    }

    {
        DropboxListFilesJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remotePath + "/file1.txt");
        job.start();
        QSignalSpy spy(&job, &DropboxListFilesJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }
}

void DropboxListFilesJobTest::cleanupTestCase() {}

QTEST_MAIN(DropboxListFilesJobTest)

#include "tst_dropboxlistfilesjob.moc"
