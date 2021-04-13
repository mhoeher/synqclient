#include <QtTest>

// add necessary includes here
#include "SynqClient/DropboxUploadFileJob"

class DropboxUploadFileJobTest : public QObject
{
    Q_OBJECT

public:
    DropboxUploadFileJobTest();
    ~DropboxUploadFileJobTest();

private slots:
    void initTestCase();
    void cleanupTestCase();
};

DropboxUploadFileJobTest::DropboxUploadFileJobTest() {}

DropboxUploadFileJobTest::~DropboxUploadFileJobTest() {}

void DropboxUploadFileJobTest::initTestCase() {}

void DropboxUploadFileJobTest::cleanupTestCase() {}

QTEST_MAIN(DropboxUploadFileJobTest)

#include "tst_dropboxuploadfilejob.moc"

