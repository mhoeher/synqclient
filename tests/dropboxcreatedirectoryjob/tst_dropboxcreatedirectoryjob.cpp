#include <QtTest>
#include <QUuid>
#include <QSignalSpy>

// add necessary includes here
#include "SynqClient/DropboxCreateDirectoryJob"
#include "SynqClient/DropboxGetFileInfoJob"
#include "../shared/utils.h"

class DropboxCreateDirectoryJobTest : public QObject
{
    Q_OBJECT

public:
    DropboxCreateDirectoryJobTest();
    ~DropboxCreateDirectoryJobTest();

private slots:
    void initTestCase();
    void mkdir();
    void mkdirRecursive();
    void mkdirExisting();
    void cleanupTestCase();
};

DropboxCreateDirectoryJobTest::DropboxCreateDirectoryJobTest() {}

DropboxCreateDirectoryJobTest::~DropboxCreateDirectoryJobTest() {}

void DropboxCreateDirectoryJobTest::initTestCase() {}

void DropboxCreateDirectoryJobTest::mkdir()
{
    if (!SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        QSKIP("No Dropbox token configured - skipping test");
    }

    QNetworkAccessManager nam;
    auto uuid = QUuid::createUuid();
    auto path = "/DropboxCreateDirectoryJobTest-mkdir-" + uuid.toString();
    auto token = SynqClient::UnitTest::getDropboxTokenFromEnv();
    SynqClient::DropboxCreateDirectoryJob mkdirJob;
    mkdirJob.setNetworkAccessManager(&nam);
    mkdirJob.setToken(token);
    mkdirJob.setPath(path);
    mkdirJob.start();
    QSignalSpy mkdirSpy(&mkdirJob, &SynqClient::AbstractJob::finished);
    QVERIFY(mkdirSpy.wait());
    QCOMPARE(mkdirJob.error(), SynqClient::JobError::NoError);

    SynqClient::DropboxGetFileInfoJob getFileInfoJob;
    getFileInfoJob.setNetworkAccessManager(&nam);
    getFileInfoJob.setToken(token);
    getFileInfoJob.setPath(path);
    getFileInfoJob.start();
    QSignalSpy getFileInfoSpy(&getFileInfoJob, &SynqClient::AbstractJob::finished);
    QVERIFY(getFileInfoSpy.wait());
    QCOMPARE(getFileInfoJob.error(), SynqClient::JobError::NoError);
    auto fileInfo = getFileInfoJob.fileInfo();
    QVERIFY(fileInfo.isValid());
    QVERIFY(fileInfo.isDirectory());
    QCOMPARE(fileInfo.name(), path.mid(1));
}

/**
 * @brief Test recursive creation of a folder in Dropbox.
 *
 * With Dropbox, we can create folders recursively without an issue. Hence, test that this
 * works as expected.
 */
void DropboxCreateDirectoryJobTest::mkdirRecursive()
{
    if (!SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        QSKIP("No Dropbox token configured - skipping test");
    }

    QNetworkAccessManager nam;
    auto uuid = QUuid::createUuid();
    auto path = "/DropboxCreateDirectoryJobTest-mkdirInvalidPath-" + uuid.toString();
    auto token = SynqClient::UnitTest::getDropboxTokenFromEnv();
    SynqClient::DropboxCreateDirectoryJob mkdirJob;
    mkdirJob.setNetworkAccessManager(&nam);
    mkdirJob.setToken(token);
    mkdirJob.setPath(path + "/sub-folder");
    mkdirJob.start();
    QSignalSpy mkdirSpy(&mkdirJob, &SynqClient::CreateDirectoryJob::finished);
    QVERIFY(mkdirSpy.wait());
    QCOMPARE(mkdirJob.error(), SynqClient::JobError::NoError);

    SynqClient::DropboxGetFileInfoJob getFileInfoJob;
    getFileInfoJob.setNetworkAccessManager(&nam);
    getFileInfoJob.setToken(token);
    getFileInfoJob.setPath(path);
    getFileInfoJob.start();
    QSignalSpy getFileInfoSpy(&getFileInfoJob, &SynqClient::AbstractJob::finished);
    QVERIFY(getFileInfoSpy.wait());
    QCOMPARE(getFileInfoJob.error(), SynqClient::JobError::NoError);
    auto fileInfo = getFileInfoJob.fileInfo();
    QVERIFY(fileInfo.isDirectory());
    QCOMPARE(fileInfo.name(), path.mid(1));
}

void DropboxCreateDirectoryJobTest::mkdirExisting()
{
    if (!SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        QSKIP("No Dropbox token configured - skipping test");
    }

    QNetworkAccessManager nam;
    auto uuid = QUuid::createUuid();
    auto path = "/DropboxCreateDirectoryJobTest-mkdir-" + uuid.toString();
    auto token = SynqClient::UnitTest::getDropboxTokenFromEnv();

    {
        SynqClient::DropboxCreateDirectoryJob mkdirJob;
        mkdirJob.setNetworkAccessManager(&nam);
        mkdirJob.setToken(token);
        mkdirJob.setPath(path);
        mkdirJob.start();
        QSignalSpy mkdirSpy(&mkdirJob, &SynqClient::DropboxCreateDirectoryJob::finished);
        QVERIFY(mkdirSpy.wait());
        QCOMPARE(mkdirJob.error(), SynqClient::JobError::NoError);
    }

    {
        SynqClient::DropboxGetFileInfoJob getFileInfoJob;
        getFileInfoJob.setNetworkAccessManager(&nam);
        getFileInfoJob.setToken(token);
        getFileInfoJob.setPath(path);
        getFileInfoJob.start();
        QSignalSpy getFileInfoSpy(&getFileInfoJob, &SynqClient::AbstractJob::finished);
        QVERIFY(getFileInfoSpy.wait());
        QCOMPARE(getFileInfoJob.error(), SynqClient::JobError::NoError);
        auto fileInfo = getFileInfoJob.fileInfo();
        QVERIFY(fileInfo.isValid());
        QVERIFY(fileInfo.isDirectory());
        QCOMPARE(fileInfo.name(), path.mid(1));
    }

    {
        SynqClient::DropboxCreateDirectoryJob mkdirJob;
        mkdirJob.setNetworkAccessManager(&nam);
        mkdirJob.setToken(token);
        mkdirJob.setPath(path);
        mkdirJob.start();
        QSignalSpy mkdirSpy(&mkdirJob, &SynqClient::DropboxCreateDirectoryJob::finished);
        QVERIFY(mkdirSpy.wait());
        QCOMPARE(mkdirJob.error(), SynqClient::JobError::FolderExists);
    }
}

void DropboxCreateDirectoryJobTest::cleanupTestCase() {}

QTEST_MAIN(DropboxCreateDirectoryJobTest)

#include "tst_dropboxcreatedirectoryjob.moc"
