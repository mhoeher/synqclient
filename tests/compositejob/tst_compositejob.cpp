#include <QTimer>
#include <QtTest>

// add necessary includes here
#include "../shared/utils.h"
#include "CompositeJob"
#include "WebDAVCreateDirectoryJob"
#include "WebDAVGetFileInfoJob"
#include "WebDAVUploadFileJob"

using SynqClient::AbstractJob;
using SynqClient::CompositeJob;
using SynqClient::CompositeJobErrorMode;
using SynqClient::JobError;
using SynqClient::JobState;

class CompositeJobTest : public QObject
{
    Q_OBJECT

public:
    CompositeJobTest();
    ~CompositeJobTest();

private slots:
    void initTestCase();
    void runBatch();
    void runSequential();
    void spawnFromJobFinished();
    void stopAfterFirstError();
    void runAllJobsDespiteOfErrors();
    void cleanupTestCase();
};

class TestJob : public AbstractJob
{
public:
    using AbstractJob::AbstractJob;

    void start() override;
    void stop() override;
    void fail();
    void setCallback(std::function<void()> cb);
    void setInterval(int interval);
    int interval() const;

private:
    QTimer m_timer;
    int m_interval = 100;
    std::function<void()> m_finishedCb;
};

CompositeJobTest::CompositeJobTest() {}

CompositeJobTest::~CompositeJobTest() {}

void CompositeJobTest::initTestCase() {}

void CompositeJobTest::runBatch()
{
    CompositeJob job;
    int numFinished = 0;
    for (int i = 0; i < 24; ++i) {
        auto j = new TestJob(&job);
        connect(j, &TestJob::finished, j, [&]() { ++numFinished; });
        job.addJob(j);
    }
    job.start();
    QSignalSpy spy(&job, &CompositeJob::finished);
    QVERIFY(spy.wait());
    QCOMPARE(job.error(), JobError::NoError);
    QCOMPARE(numFinished, 24);
}

void CompositeJobTest::runSequential()
{
    CompositeJob job;
    job.setMaxJobs(1);
    int numFinished = 0;
    for (int i = 0; i < 3; ++i) {
        auto j = new TestJob(&job);
        connect(j, &TestJob::finished, j, [&]() { ++numFinished; });
        job.addJob(j);
    }
    job.start();
    QSignalSpy spy(&job, &CompositeJob::finished);
    QVERIFY(spy.wait());
    QCOMPARE(job.error(), JobError::NoError);
    QCOMPARE(numFinished, 3);
}

void CompositeJobTest::spawnFromJobFinished()
{
    CompositeJob job;
    job.setMaxJobs(1);
    int numFinished = 0;
    std::function<void()> createJobFn;
    createJobFn = [&]() {
        auto j = new TestJob(&job);
        job.addJob(j);
        connect(j, &TestJob::finished, [&]() {
            ++numFinished;
            if (numFinished < 5) {
                createJobFn();
            }
        });
    };
    createJobFn();
    job.start();
    QSignalSpy spy(&job, &CompositeJob::finished);
    QVERIFY(spy.wait());
    QCOMPARE(job.error(), JobError::NoError);
    QCOMPARE(numFinished, 5);
}

void CompositeJobTest::stopAfterFirstError()
{
    CompositeJob job;
    job.setMaxJobs(5);
    job.setErrorMode(CompositeJobErrorMode::StopOnFirstError);
    int numFinished = 0;
    int numStopped = 0;
    for (int i = 0; i < 20; ++i) {
        auto j = new TestJob(&job);
        if (i < 5) {
            j->setInterval(100);
        } else {
            j->setInterval(500);
        }
        if (i == 4) {
            j->fail();
            j->setInterval(200);
        }
        connect(j, &TestJob::finished, this, [&numFinished, &numStopped, j]() {
            if (j->error() != JobError::Stopped) {
                ++numFinished;
            } else {
                ++numStopped;
            }
        });
        job.addJob(j);
    }
    job.start();
    QSignalSpy spy(&job, &CompositeJob::finished);
    QVERIFY(spy.wait());
    QCOMPARE(job.error(), JobError::InvalidParameter);
    QCOMPARE(numFinished, 5);
    QCOMPARE(numStopped, 4);
}

void CompositeJobTest::runAllJobsDespiteOfErrors()
{
    CompositeJob job;
    job.setMaxJobs(5);
    job.setErrorMode(CompositeJobErrorMode::RunAllJobs);
    int numFinished = 0;
    int numStopped = 0;
    for (int i = 0; i < 20; ++i) {
        auto j = new TestJob(&job);
        if (i < 5) {
            j->setInterval(100);
        } else {
            j->setInterval(500);
        }
        if (i == 4) {
            j->fail();
            j->setInterval(200);
        }
        connect(j, &TestJob::finished, this, [&numFinished, &numStopped, j]() {
            if (j->error() != JobError::Stopped) {
                ++numFinished;
            } else {
                ++numStopped;
            }
        });
        job.addJob(j);
    }
    job.start();
    QSignalSpy spy(&job, &CompositeJob::finished);
    QVERIFY(spy.wait());
    QCOMPARE(job.error(), JobError::InvalidParameter);
    QCOMPARE(numFinished, 20);
    QCOMPARE(numStopped, 0);
}

void CompositeJobTest::cleanupTestCase() {}

void TestJob::start()
{
    if (state() == JobState::Ready) {
        setState(JobState::Running);
        m_timer.setInterval(m_interval);
        m_timer.setSingleShot(true);
        connect(&m_timer, &QTimer::timeout, this, [=]() {
            if (state() == JobState::Running) {
                if (m_finishedCb) {
                    m_finishedCb();
                }
                finishLater();
            }
        });
        m_timer.start();
    }
}

void TestJob::stop()
{
    setError(JobError::Stopped, tr("Stopped"));
    finishLater();
    m_timer.stop();
}

void TestJob::fail()
{
    setError(JobError::InvalidParameter, tr("Test Error"));
}

void TestJob::setCallback(std::function<void()> cb)
{
    m_finishedCb = cb;
}

void TestJob::setInterval(int interval)
{
    m_interval = interval;
}

int TestJob::interval() const
{
    return m_interval;
}

QTEST_MAIN(CompositeJobTest)

#include "tst_compositejob.moc"
