/*
 * Copyright 2021 Martin Hoeher <martin@rpdev.net>
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

#include "../inc/compositejob.h"

#include <QTimer>

#include "compositejobprivate.h"

namespace SynqClient {

/**
 * @brief Constructor.
 */
CompositeJob::CompositeJob(QObject* parent) : AbstractJob(new CompositeJobPrivate(this), parent) {}

/**
 * @brief Destructor.
 */
CompositeJob::~CompositeJob() {}

/**
 * @brief The maximum number of jobs that are run in parallel.
 *
 * This property holds the maximum number of jobs that the composite will run in parallel. Running
 * multiple jobs at once can be beneficial, e.g. by usitlizing parallelism and pipelining when
 * accessing a remote server. Setting this property to `1` effectively causes the composite to run
 * all child jobs sequentially.
 *
 * By default, 12 jobs are run in parallel. This is double the number of network connections, a
 * QNetworkAccessManager will spawn in parallel and hence should make optimal use of parallelism and
 * pipelining (especially if additional actions are run in the finished handler of the child jobs).
 *
 * @note If the remote server requires sequential accesses, this value must be set to `1`.
 */
int CompositeJob::maxJobs() const
{
    Q_D(const CompositeJob);
    return d->maxJobs;
}

/**
 * @brief Set the maximum number of jobs run in parallel to @p maxJobs.
 */
void CompositeJob::setMaxJobs(int maxJobs)
{
    Q_D(CompositeJob);
    if (maxJobs < 1) {
        maxJobs = 1;
    }
    d->maxJobs = maxJobs;
}

/**
 * @brief Determines how child job errors are handled.
 *
 * This option controls the behaviour in case a child job has an error. The default is
 * CompositeJobErrorMode::RunAllJobs.
 */
CompositeJobErrorMode CompositeJob::errorMode() const
{
    Q_D(const CompositeJob);
    return d->errorMode;
}

/**
 * @brief Set the @p errorMode to be used.
 */
void CompositeJob::setErrorMode(CompositeJobErrorMode errorMode)
{
    Q_D(CompositeJob);
    d->errorMode = errorMode;
}

/**
 * @brief Add a child job to the composite.
 *
 * This adds the given child @p job to this composite job. This function can be called before
 * starting the composite job or also while it is running. The composite - once started - will start
 * all child jobs and wait for them to finish.
 *
 * @note The composite will not take ownership of the job. It is up to the user to ensure, that jobs
 * are eventually deleted. Ideally, connect the finished signal of the child job to
 * QObject::deleteLater, so it is eventually deleted once it finished. The composite will detect
 * deletions. However, if a job is deleted before it is finished (or after it finished but before
 * the composite's handler of that even is called), it won't notice if the job finished with an
 * error. Hence, the configured errorMode() might be ignored. If you delete a job by wiring its
 * finished signal to deleteLater, this should not be an issue.
 *
 * @warning Jobs which are in the JobState::Ready state are ignored.
 */
void CompositeJob::addJob(AbstractJob* job)
{
    Q_D(CompositeJob);
    if (job && job->state() != JobState::Finished) {
        d->childJobs.append(QPointer<AbstractJob>(job));
        connect(job, &AbstractJob::finished, this, [=]() {
            // Eventually check for running jobs:
            QTimer::singleShot(0, this, [=]() { d->runJobs(); });
        });
    }
}

/**
 * @brief Implementation of AbstractJob::start().
 */
void CompositeJob::start()
{
    Q_D(CompositeJob);
    if (d->state != JobState::Ready) {
        return;
    }
    d->runJobs();
}

/**
 * @brief Implementation of AbstractJob::stop().
 */
void CompositeJob::stop()
{
    Q_D(CompositeJob);

    // Set error:
    setError(JobError::Stopped, tr("The composite job has been stopped"));

    // Stop all children:
    bool hasRunningJobs = false;
    for (auto& job : d->childJobs) {
        if (job && job->state() == JobState::Running) {
            job->stop();
            hasRunningJobs = true;
        }
    }

    // If we don't have any running jobs, terminate right away:
    if (!hasRunningJobs) {
        finishLater();
    }
}

/**
 * @brief Constructor.
 */
CompositeJob::CompositeJob(CompositeJobPrivate* d, QObject* parent) : AbstractJob(d, parent) {}

} // namespace SynqClient
