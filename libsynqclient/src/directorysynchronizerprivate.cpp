/*
 * Copyright 2020 Martin Hoeher <martin@rpdev.net>
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

#include "directorysynchronizerprivate.h"

#include <QTimer>

#include "abstractjobfactory.h"
#include "syncstatedatabase.h"

namespace SynqClient {

DirectorySynchronizerPrivate::DirectorySynchronizerPrivate(DirectorySynchronizer* q)
    : q_ptr(q),
      jobFactory(nullptr),
      syncStateDatabase(nullptr),
      localDirectoryPath(),
      remoteDirectoryPath(),
      filter([](const QString&, const FileInfo&) { return true; }),
      state(SynchronizerState::Ready),
      error(SynchronizerError::NoError),
      maxJobs(12),
      syncConflictStrategy(SyncConflictStrategy::RemoteWins),
      flags(SynchronizerFlag::DefaultFlags)
{
}

void DirectorySynchronizerPrivate::finishLater()
{
    Q_Q(DirectorySynchronizer);
    QTimer::singleShot(0, q, [=] {
        state = SynchronizerState::Finished;
        emit q->finished();
    });
}

} // namespace SynqClient
