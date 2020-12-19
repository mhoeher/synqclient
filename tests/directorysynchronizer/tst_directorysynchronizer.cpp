#include <QtTest>

// add necessary includes here
#include "DirectorySynchronizer"

class DirectorySynchronizerTest : public QObject
{
    Q_OBJECT

public:
    DirectorySynchronizerTest();
    ~DirectorySynchronizerTest();

private slots:
    void initTestCase();
    void cleanupTestCase();
};

DirectorySynchronizerTest::DirectorySynchronizerTest() {}

DirectorySynchronizerTest::~DirectorySynchronizerTest() {}

void DirectorySynchronizerTest::initTestCase() {}

void DirectorySynchronizerTest::cleanupTestCase() {}

QTEST_MAIN(DirectorySynchronizerTest)

#include "tst_directorysynchronizer.moc"

