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

#include "compositejobprivate.h"

namespace SynqClient {

CompositeJobPrivate::CompositeJobPrivate(CompositeJob* q)
    : AbstractJobPrivate(q),
      childJobs(),
      maxJobs(12),
      errorMode(CompositeJobErrorMode::RunAllJobs),
      firstChildError(JobError::NoError),
      firstChildErrorString(),
      stopRunningChildrenAndExit(false)
{
}

void CompositeJobPrivate::runJobs()
{
    Q_Q(CompositeJob);

    if (error != JobError::NoError && error != JobError::Stopped) {
        q->finishLater();
        return;
    }

    // Check if we have been stopped:
    if (error == JobError::Stopped) {
        // Yes. Check if we still have running jobs. If so, we still need to wait:
        if (std::any_of(childJobs.cbegin(), childJobs.cend(), [=](QPointer<AbstractJob> job) {
                return job && job->state() == JobState::Running;
            })) {
            return;
        } else {
            q->finishLater();
        }
    }

    // Collect all jobs which are not yet deleted
    QVector<QPointer<AbstractJob>> waitingJobs;
    decltype(childJobs) remainingJobs;
    waitingJobs.reserve(childJobs.length());
    for (const auto& job : qAsConst(childJobs)) {
        if (job) {
            switch (job->state()) {
            case JobState::Ready:
                waitingJobs << job;
                break;
            case JobState::Running:
                remainingJobs << job;
                break;
            case JobState::Finished:
                if (job->error() != JobError::NoError) {
                    // The job had an error. Record, if it is the first one:
                    if (firstChildError == JobError::NoError) {
                        firstChildError = job->error();
                        firstChildErrorString =
                                tr("Child job encountered an error: %1").arg(job->errorString());
                    }
                    switch (errorMode) {
                    case CompositeJobErrorMode::RunAllJobs:
                        // Continue
                        break;
                    case CompositeJobErrorMode::StopOnFirstError:
                        // Stop running children (later).
                        stopRunningChildrenAndExit = true;
                        break;
                    }
                }
            }
        }
    }

    // Did we have an error? Stop running jobs and wait for them to finish:
    if (stopRunningChildrenAndExit) {
        for (const auto& job : qAsConst(remainingJobs)) {
            job->stop();
        }
    } else {
        // If we are here, everything is fine. In this case, spawn additional jobs until the
        // maximum number of running jobs is reached:
        for (const auto& job : qAsConst(waitingJobs)) {
            if (remainingJobs.length() < maxJobs) {
                job->start();
            }
            remainingJobs << job;
        }
    }

    if (remainingJobs.isEmpty()) {
        // No jobs remaining. Propagate child error (if any) and finish:
        if (error == JobError::NoError) {
            q->setError(firstChildError, firstChildErrorString);
        }
        q->finishLater();
    }

    childJobs = remainingJobs;
}

} // namespace SynqClient
