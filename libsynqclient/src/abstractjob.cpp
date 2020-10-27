#include "../inc/abstractjob.h"

#include <QTimer>

#include "abstractjobprivate.h"

namespace SynqClient {

AbstractJob::AbstractJob(QObject* parent) : QObject(parent), d_ptr(new AbstractJobPrivate(this)) {}

AbstractJob::~AbstractJob() {}

JobError AbstractJob::error() const
{
    Q_D(const AbstractJob);
    return d->error;
}

QString AbstractJob::errorString() const
{
    Q_D(const AbstractJob);
    return d->errorString;
}

JobState AbstractJob::state() const
{
    Q_D(const AbstractJob);
    return d->state;
}

AbstractJob::AbstractJob(AbstractJobPrivate* d, QObject* parent) : QObject(parent), d_ptr(d) {}

void AbstractJob::setError(JobError error, const QString& errorString)
{
    Q_D(AbstractJob);
    d->error = error;
    d->errorString = errorString;
}

void AbstractJob::setState(JobState state)
{
    Q_D(AbstractJob);
    d->state = state;
}

/**
 * @brief Emit the finished signal once control returns to the event loop.
 *
 * This method can be used to emit the finished signal once control returns to the
 * event loop. Sub-classes shall use this method to emit the finished() signal later.
 * Usually, this can be called in the start() implementation when doing error
 * checking:
 *
 * @code
 * void MyJob::start()
 * {
 *     if (checkForJobParameters()) {
 *         startProcessThread();
 *     } else {
 *         setError(JobError::MissingParameter, "The job is missing some parameter(s)");
 *         finishLater();
 *     }
 * }
 * @endcode
 *
 * It is important to not emit the finished signal directly in the start() method
 * as otherwise code interfacing would need to check twice for whether the job
 * finished (directly after calling start as well as when the job really finished).
 */
void AbstractJob::finishLater()
{
    Q_D(AbstractJob);
    auto timer = new QTimer(this);
    timer->setInterval(0);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, [=]() {
        d->state = JobState::Finished;
        emit finished();
        timer->deleteLater();
    });
    timer->start();
}

} // namespace SynqClient
