#include <QtTest>

// add necessary includes here
#include "SynqClient/DropboxGetFileInfoJob"
#include "../shared/utils.h"

class DropboxGetFileInfoJobTest : public QObject
{
    Q_OBJECT

public:
    DropboxGetFileInfoJobTest();
    ~DropboxGetFileInfoJobTest();

private slots:
    void initTestCase();
    void getRootItemFileInfo();
    void getFileInfoForNonExistingFile();
    void cleanupTestCase();
};

DropboxGetFileInfoJobTest::DropboxGetFileInfoJobTest() {}

DropboxGetFileInfoJobTest::~DropboxGetFileInfoJobTest() {}

void DropboxGetFileInfoJobTest::initTestCase() {}

/**
 * @brief Get file information for the root folder.
 *
 * This test should fail - Dropbox does not allow fetching metadata for the root folder.
 */
void DropboxGetFileInfoJobTest::getRootItemFileInfo()
{
    if (!SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        QSKIP("No Dropbox token configured - skipping test");
    }

    QNetworkAccessManager nam;
    SynqClient::DropboxGetFileInfoJob job;
    job.setNetworkAccessManager(&nam);
    job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
    job.start();
    QSignalSpy spy(&job, &SynqClient::AbstractJob::finished);
    QVERIFY(spy.wait());
    QCOMPARE(job.error(), SynqClient::JobError::NetworkRequestFailed);
    auto fileInfo = job.fileInfo();
    QVERIFY(!fileInfo.isValid());
}

void DropboxGetFileInfoJobTest::getFileInfoForNonExistingFile()
{
    if (!SynqClient::UnitTest::hasDropboxTokenFromEnv()) {
        QSKIP("No Dropbox token configured - skipping test");
    }

    auto uid = QUuid::createUuid();
    auto path = "/DropboxGetFileInfoJobTest-should-definitely-not-exist-" + uid.toString();
    QNetworkAccessManager nam;
    SynqClient::DropboxGetFileInfoJob job;
    job.setNetworkAccessManager(&nam);
    job.setToken(SynqClient::UnitTest::getDropboxTokenFromEnv());
    job.setPath(path);
    job.start();
    QSignalSpy spy(&job, &SynqClient::AbstractJob::finished);
    QVERIFY(spy.wait());
    QCOMPARE(job.error(), SynqClient::JobError::ResourceNotFound);
    auto fileInfo = job.fileInfo();
    QVERIFY(!fileInfo.isValid());
}

void DropboxGetFileInfoJobTest::cleanupTestCase() {}

QTEST_MAIN(DropboxGetFileInfoJobTest)

#include "tst_dropboxgetfileinfojob.moc"
