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

#ifndef SYNQCLIENT_SYNCSTATEENTRY_H
#define SYNQCLIENT_SYNCSTATEENTRY_H

#include <QObject>
#include <QScopedPointer>
#include <QSharedDataPointer>
#include <QtGlobal>

#include "libsynqclient_global.h"

namespace SynqClient {

class SyncStateEntryPrivate;

class LIBSYNQCLIENT_EXPORT SyncStateEntry
{
public:
    SyncStateEntry();
    SyncStateEntry(const SyncStateEntry& other);
    virtual ~SyncStateEntry();

    bool isValid() const;
    void setValid(bool valid);

    QString path() const;
    void setPath(const QString& path);

    QDateTime modificationTime() const;
    void setModificationTime(const QDateTime& modificationTime);

    QString syncProperty() const;
    void setSyncProperty(const QString& syncProperty);

protected:
    explicit SyncStateEntry(SyncStateEntryPrivate* d);

    QSharedDataPointer<SyncStateEntryPrivate> d;
};

} // namespace SynqClient

#endif // SYNQCLIENT_SYNCSTATEENTRY_H
