#include <QtTest>

// add necessary includes here
#include "SynqClient/DropboxGetFileInfoJob"

class DropboxGetFileInfoJobTest : public QObject
{
    Q_OBJECT

public:
    DropboxGetFileInfoJobTest();
    ~DropboxGetFileInfoJobTest();

private slots:
    void initTestCase();
    void cleanupTestCase();
};

DropboxGetFileInfoJobTest::DropboxGetFileInfoJobTest() {}

DropboxGetFileInfoJobTest::~DropboxGetFileInfoJobTest() {}

void DropboxGetFileInfoJobTest::initTestCase() {}

void DropboxGetFileInfoJobTest::cleanupTestCase() {}

QTEST_MAIN(DropboxGetFileInfoJobTest)

#include "tst_dropboxgetfileinfojob.moc"

