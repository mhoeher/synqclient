#include <QtTest>

// add necessary includes here
#include "../shared/utils.h"
#include "SynqClient/DropboxCreateDirectoryJob"
#include "SynqClient/DropboxDeleteJob"
#include "SynqClient/DropboxGetFileInfoJob"
#include "SynqClient/DropboxListFilesJob"
#include "SynqClient/DropboxUploadFileJob"

using SynqClient::DropboxCreateDirectoryJob;
using SynqClient::DropboxDeleteJob;
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
    void listFilesRecursively();
    void listFilesWithCursor();
    void listFilesRecursivelyWithCursor();
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

void DropboxListFilesJobTest::listFilesRecursively()
{
    if (!SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        QSKIP("No Dropbox token configured - skipping test");
    }

    QNetworkAccessManager nam;
    nam.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/DropboxListFilesJobTest-listFilesRecursively-" + testDirUid.toString();

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
        job.setRemoteFilename(remotePath + "/dir1/file1.txt");
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
        job.setRemoteFilename(remotePath + "/dir1/file2.txt");
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
        job.setRecursive(true);
        job.start();
        QSignalSpy spy(&job, &DropboxListFilesJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
        QCOMPARE(job.entries().length(), 4);

        QStringList expectedNames { "dir1", "dir1/file1.txt", "dir1/file2.txt", "dir2" };
        QStringList gotNames;
        for (const auto& entry : job.entries()) {
            auto name = entry.path();
            gotNames << name;
            if (name == "dir1" || name == "dir2") {
                QVERIFY(entry.isDirectory());
            }
            if (name == "dir1/file1.txt" || name == "dir1/file2.txt") {
                QVERIFY(entry.isFile());
            }
        }
        std::sort(gotNames.begin(), gotNames.end());
        QCOMPARE(gotNames, expectedNames);
        QVERIFY(job.folder().isValid());
        QCOMPARE(job.folder().name(), ".");
        QVERIFY(job.folder().isDirectory());
    }
}

void DropboxListFilesJobTest::listFilesWithCursor()
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

    QString cursor;

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
        cursor = job.cursor();
    }

    {
        DropboxUploadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setRemoteFilename(remotePath + "/file3.txt");
        job.setData("Hello World again!");
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
        job.setCursor(cursor);
        job.start();
        QSignalSpy spy(&job, &DropboxListFilesJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
        QCOMPARE(job.entries().length(), 1);
        auto entry = job.entries().at(0);
        QCOMPARE(entry.name(), "file3.txt");
        QVERIFY(entry.isFile());
        cursor = job.cursor();
    }

    {
        DropboxUploadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setRemoteFilename(remotePath + "/file3.txt");
        job.setData("Hello World modified!");
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
        job.setCursor(cursor);
        job.start();
        QSignalSpy spy(&job, &DropboxListFilesJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
        QCOMPARE(job.entries().length(), 1);
        auto entry = job.entries().at(0);
        QCOMPARE(entry.name(), "file3.txt");
        QVERIFY(entry.isFile());
        cursor = job.cursor();
    }

    {
        DropboxCreateDirectoryJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remotePath + "/dir3");
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
        job.setCursor(cursor);
        job.start();
        QSignalSpy spy(&job, &DropboxListFilesJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
        QCOMPARE(job.entries().length(), 1);
        auto entry = job.entries().at(0);
        QCOMPARE(entry.name(), "dir3");
        QVERIFY(entry.isDirectory());
        cursor = job.cursor();
    }

    {
        DropboxDeleteJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remotePath + "/dir3");
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
        job.setCursor(cursor);
        job.start();
        QSignalSpy spy(&job, &DropboxListFilesJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
        QCOMPARE(job.entries().length(), 1);
        auto entry = job.entries().at(0);
        QCOMPARE(entry.name(), "dir3");
        QVERIFY(entry.isDeleted());
        cursor = job.cursor();
    }

    {
        DropboxDeleteJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remotePath + "/file3.txt");
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
        job.setCursor(cursor);
        job.start();
        QSignalSpy spy(&job, &DropboxListFilesJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
        QCOMPARE(job.entries().length(), 1);
        auto entry = job.entries().at(0);
        QCOMPARE(entry.name(), "file3.txt");
        QVERIFY(entry.isDeleted());
        cursor = job.cursor();
    }
}

void DropboxListFilesJobTest::listFilesRecursivelyWithCursor()
{
    if (!SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        QSKIP("No Dropbox token configured - skipping test");
    }

    QNetworkAccessManager nam;
    nam.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    auto testDirUid = QUuid::createUuid();
    auto remotePath = "/DropboxListFilesJobTest-listFilesRecursively-" + testDirUid.toString();

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
        job.setRemoteFilename(remotePath + "/dir1/file1.txt");
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
        job.setRemoteFilename(remotePath + "/dir1/file2.txt");
        job.setData("Another file!");
        job.start();
        QSignalSpy spy(&job, &DropboxUploadFileJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
    }

    QString cursor;

    {
        DropboxListFilesJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remotePath);
        job.setRecursive(true);
        job.start();
        QSignalSpy spy(&job, &DropboxListFilesJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
        QCOMPARE(job.entries().length(), 4);
        QVERIFY(!job.incremental());
        cursor = job.cursor();
    }

    {
        DropboxUploadFileJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setRemoteFilename(remotePath + "/dir1/file3.txt");
        job.setData("Hello World #3!");
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
        job.setCursor(cursor);
        job.setRecursive(true);
        job.start();
        QSignalSpy spy(&job, &DropboxListFilesJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
        QCOMPARE(job.entries().length(), 1);
        auto entry = job.entries().at(0);
        QCOMPARE(entry.name(), "file3.txt");
        QCOMPARE(entry.path(), "dir1/file3.txt");
        QVERIFY(entry.isFile());
        cursor = job.cursor();
    }

    {
        DropboxCreateDirectoryJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remotePath + "/dir2/dir2.1");
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
        job.setCursor(cursor);
        job.setRecursive(true);
        job.start();
        QSignalSpy spy(&job, &DropboxListFilesJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
        QCOMPARE(job.entries().length(), 1);
        auto entry = job.entries().at(0);
        QCOMPARE(entry.name(), "dir2.1");
        QCOMPARE(entry.path(), "dir2/dir2.1");
        QVERIFY(entry.isDirectory());
        cursor = job.cursor();
    }

    {
        DropboxCreateDirectoryJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remotePath + "/dir2/dir2.2");
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
        job.setCursor(cursor);
        job.setRecursive(true);
        job.start();
        QSignalSpy spy(&job, &DropboxListFilesJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
        QCOMPARE(job.entries().length(), 1);
        auto entry = job.entries().at(0);
        QCOMPARE(entry.name(), "dir2.2");
        QCOMPARE(entry.path(), "dir2/dir2.2");
        QVERIFY(entry.isDirectory());
        QVERIFY(job.incremental());
        cursor = job.cursor();
    }

    {
        DropboxDeleteJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remotePath + "/dir2/dir2.2");
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
        job.setCursor(cursor);
        job.setRecursive(true);
        job.start();
        QSignalSpy spy(&job, &DropboxListFilesJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
        QCOMPARE(job.entries().length(), 1);
        QVERIFY(job.incremental());
        auto entry = job.entries().at(0);
        QCOMPARE(entry.name(), "dir2.2");
        QCOMPARE(entry.path(), "dir2/dir2.2");
        QVERIFY(entry.isDeleted());
        cursor = job.cursor();
    }

    {
        DropboxDeleteJob job;
        job.setNetworkAccessManager(&nam);
        job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
        job.setPath(remotePath + "/dir2");
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
        job.setCursor(cursor);
        job.setRecursive(true);
        job.start();
        QSignalSpy spy(&job, &DropboxListFilesJob::finished);
        QVERIFY(spy.wait());
        QCOMPARE(job.error(), JobError::NoError);
        QCOMPARE(job.entries().length(), 2);
        QVERIFY(job.incremental());
        for (const auto& entry : job.entries()) {
            QVERIFY(entry.name() == "dir2" || entry.name() == "dir2.1");
            QVERIFY(entry.path() == "dir2" || entry.path() == "dir2/dir2.1");
            QVERIFY(entry.isDeleted());
        }
        QVERIFY(job.entries().at(0).path() != job.entries().at(1).path());
        cursor = job.cursor();
    }
}

void DropboxListFilesJobTest::cleanupTestCase() {}

QTEST_MAIN(DropboxListFilesJobTest)

#include "tst_dropboxlistfilesjob.moc"
