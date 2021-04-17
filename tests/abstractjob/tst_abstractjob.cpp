#include <QSignalSpy>
#include <QTimer>
#include <QtTest>

// add necessary includes here
#include "SynqClient/AbstractJob"

class AbstractJobTest : public QObject
{
    Q_OBJECT

public:
    AbstractJobTest();
    ~AbstractJobTest();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void state();
    void error();
};

class Dummy : public SynqClient::AbstractJob
{
public:
    void start() override { setState(SynqClient::JobState::Running); };
    void stop() override
    {
        setState(SynqClient::JobState::Finished);
        setError(SynqClient::JobError::Stopped, "The job has been stopped by the user");
        finishLater();
    };
};

AbstractJobTest::AbstractJobTest() {}

AbstractJobTest::~AbstractJobTest() {}

void AbstractJobTest::initTestCase() {}

void AbstractJobTest::cleanupTestCase() {}

void AbstractJobTest::state()
{
    Dummy d;
    QCOMPARE(d.state(), SynqClient::JobState::Ready);
    d.start();
    QCOMPARE(d.state(), SynqClient::JobState::Running);
    d.stop();
    QSignalSpy finished(&d, &Dummy::finished);
    QVERIFY(finished.wait());
    QCOMPARE(d.state(), SynqClient::JobState::Finished);
}

void AbstractJobTest::error()
{
    Dummy d;
    QCOMPARE(d.error(), SynqClient::JobError::NoError);
    QCOMPARE(d.errorString(), QString());
    d.start();
    QCOMPARE(d.error(), SynqClient::JobError::NoError);
    QCOMPARE(d.errorString(), QString());
    d.stop();
    QSignalSpy finished(&d, &Dummy::finished);
    QVERIFY(finished.wait());
    QCOMPARE(d.error(), SynqClient::JobError::Stopped);
    QCOMPARE(d.errorString(), "The job has been stopped by the user");
}

QTEST_MAIN(AbstractJobTest)

#include "tst_abstractjob.moc"
