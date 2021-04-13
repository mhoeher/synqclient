#include <QtTest>

// add necessary includes here
#include "SynqClient/DropboxDeleteJob"

class DropboxDeleteJobTest : public QObject
{
    Q_OBJECT

public:
    DropboxDeleteJobTest();
    ~DropboxDeleteJobTest();

private slots:
    void initTestCase();
    void cleanupTestCase();
};

DropboxDeleteJobTest::DropboxDeleteJobTest() {}

DropboxDeleteJobTest::~DropboxDeleteJobTest() {}

void DropboxDeleteJobTest::initTestCase() {}

void DropboxDeleteJobTest::cleanupTestCase() {}

QTEST_MAIN(DropboxDeleteJobTest)

#include "tst_dropboxdeletejob.moc"

