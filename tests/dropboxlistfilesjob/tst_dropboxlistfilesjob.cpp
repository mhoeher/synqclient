#include <QtTest>

// add necessary includes here
#include "SynqClient/DropboxListFilesJob"

class DropboxListFilesJobTest : public QObject
{
    Q_OBJECT

public:
    DropboxListFilesJobTest();
    ~DropboxListFilesJobTest();

private slots:
    void initTestCase();
    void cleanupTestCase();
};

DropboxListFilesJobTest::DropboxListFilesJobTest() {}

DropboxListFilesJobTest::~DropboxListFilesJobTest() {}

void DropboxListFilesJobTest::initTestCase() {}

void DropboxListFilesJobTest::cleanupTestCase() {}

QTEST_MAIN(DropboxListFilesJobTest)

#include "tst_dropboxlistfilesjob.moc"

