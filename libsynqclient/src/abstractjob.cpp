/*
 * Copyright 2020-2021 Martin Hoeher <martin@rpdev.net>
 *
 * This file is part of SynqClient.
 *
 * SynqClient is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * SynqClient is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SynqClient.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "SynqClient/abstractjob.h"

#include <QTimer>
#include <QNetworkReply>

#include "abstractjobprivate.h"

namespace SynqClient {

/**
 * @class AbstractJob
 * @brief Base class for all jobs.
 *
 * This class is the base class for all of the jobs provided by the library. On its own, it provides
 * only minimal functionality. Its main purpose is to provide a suitable interface for all concrete
 * jobs and defining the behaviour of jobs.
 *
 * # Job Life Cycle
 *
 * Jobs have a well defined sequence of states they run through during their usage. The current
 * state is returned by calling the state() method. After creating an instance of a job class, it is
 * in the JobState::Ready state. In this state, the job should be set up, i.e. essential properties
 * need to be set (e.g. for an upload job, we need to supply the data to be uploaded as well as the
 * target path where to upload data to).
 *
 * After the job has been set up, the start() method is called to proceed. This will bring the job
 * into the JobState::Running state. Note that the start() method returns immediately. Also note
 * that even if errors are already detected when calling start(), the job still first transitions to
 * the JobState::Running state and only transitions further as soon as control goes back to the
 * event loop.
 *
 * Eventually, the finished() signal will be emitted and the job transitions to the
 * JobState::Finished state.
 *
 * Note that a job object must not be reused. Once it ran through this sequence, it has to be
 * deleted.
 *
 * A running job can be terminated by calling the stop() method: This will abort running transfers
 * and also eventually cause the finished() signal to be emitted.
 *
 * # Error Handling
 *
 * Jobs use the error() method to provide information about their condition. If during execution of
 * a job everything works well, this will be set to JobError::NoError. However, if something unusual
 * happens, the error is set accordingly. After a job finished, always check the error() to see if
 * everything worked.
 *
 * Depending on the concrete job, errors might not be *fatal*. For example, a job which creates a
 * remote folder might terminate with the JobError::FolderExists code, which indicates that the
 * remote folder already exists.
 *
 * In particular, if a job is stopped by calling the stop() method, it will have an error of
 * JobError::Stopped.
 *
 */

/**
 * @fn AbstractJob::start()
 * @brief Start the job.
 *
 * This method is used to start the job after it has been set up. It only has an effect when being
 * called while the job is in the JobState::Ready state. Calling this method will cause the job to
 * transition to JobState::Running. Eventually, finished() will be emitted to indicate the job
 * finished. Check the error() to learn if the job succeeded or not.
 *
 * Calling this method while the job is in any other state has no effect.
 */

/**
 * @fn AbstractJob::stop()
 * @brief Stop a running job.
 *
 * If this is called while the job is in the JobState::Running state, the job will eventually be
 * stopped and the finished() signal be emitted. A job which has been terminated like that will have
 * an error() of JobError::Stopped.
 *
 * Calling this method while the job is in any other state has no effect.
 */

/**
 * @fn AbstractJob::finished()
 * @brief The job has finished.
 *
 * This signal is emitted once the job transitions to the JobState::Finished state. After this
 * signal is emitted, check the error() to learn if the job succeeded. Afterwards, the job can be
 * deleted.
 *
 * @note Do not reuse jobs. For each transaction you want to run, a new job object must be created.
 */

/**
 * @brief Constructor.
 */
AbstractJob::AbstractJob(QObject* parent) : QObject(parent), d_ptr(new AbstractJobPrivate(this)) {}

/**
 * @brief Destructor.
 */
AbstractJob::~AbstractJob() {}

/**
 * @brief Get the error code of the job.
 *
 * This returns a value indicating if the job had an error during execution.
 */
JobError AbstractJob::error() const
{
    Q_D(const AbstractJob);
    return d->error;
}

/**
 * @brief A textual representation of the error the job encountered.
 *
 * This returns a textual description of the error reason. Depending on the concrete error that
 * occurred, this might hold additional information to understand why the job failed.
 */
QString AbstractJob::errorString() const
{
    Q_D(const AbstractJob);
    return d->errorString;
}

/**
 * @brief The current state of the job.
 */
JobState AbstractJob::state() const
{
    Q_D(const AbstractJob);
    return d->state;
}

/**
 * @brief Constructor.
 */
AbstractJob::AbstractJob(AbstractJobPrivate* d, QObject* parent) : QObject(parent), d_ptr(d) {}

/**
 * @brief Mark the job as failed.
 *
 * This sets the @p error and @p errorString of the job. Concrete subclasses shall use this to mark
 * the job as failed if they encounter an error during execution.
 */
void AbstractJob::setError(JobError error, const QString& errorString)
{
    Q_D(AbstractJob);
    d->error = error;
    d->errorString = errorString;
}

/**
 * @brief Set the job state.
 *
 * This sets the @p state of the job to the specified value. This method shall be used by concrete
 * subclasses to progress the job through the usual lifecycle.
 */
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

/**
 * @brief Map network errors to job errors.
 *
 * This utility method checks the network @p reply and maps its error code
 * to a JobError. It can be used by jobs using Qt's *QNetworkAccessManager* API to map Qt's errors
 * consistently to job errors.
 */
JobError AbstractJob::fromNetworkError(const QNetworkReply& reply)
{
    switch (reply.error()) {
    case QNetworkReply::NoError:
        return JobError::NoError;
    case QNetworkReply::ContentConflictError:
        return JobError::ServerContentConflict;
    case QNetworkReply::ContentNotFoundError:
        return JobError::ResourceNotFound;
    default:
        return JobError::NetworkRequestFailed;
    }
}

} // namespace SynqClient
