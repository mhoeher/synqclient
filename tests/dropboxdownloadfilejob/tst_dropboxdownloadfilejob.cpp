#include <QtTest>

// add necessary includes here
#include "SynqClient/DropboxDownloadFileJob"

class DropboxDownloadFileJobTest : public QObject
{
    Q_OBJECT

public:
    DropboxDownloadFileJobTest();
    ~DropboxDownloadFileJobTest();

private slots:
    void initTestCase();
    void cleanupTestCase();
};

DropboxDownloadFileJobTest::DropboxDownloadFileJobTest() {}

DropboxDownloadFileJobTest::~DropboxDownloadFileJobTest() {}

void DropboxDownloadFileJobTest::initTestCase() {}

void DropboxDownloadFileJobTest::cleanupTestCase() {}

QTEST_MAIN(DropboxDownloadFileJobTest)

#include "tst_dropboxdownloadfilejob.moc"

