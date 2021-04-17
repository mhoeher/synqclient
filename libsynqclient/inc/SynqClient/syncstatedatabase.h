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

#ifndef SYNQCLIENT_SYNCSTATEDATABASE_H
#define SYNQCLIENT_SYNCSTATEDATABASE_H

#include <functional>

#include <QObject>
#include <QScopedPointer>
#include <QVector>
#include <QtGlobal>

#include "libsynqclient_global.h"
#include "SyncStateEntry"

namespace SynqClient {

class SyncStateDatabasePrivate;

class LIBSYNQCLIENT_EXPORT SyncStateDatabase : public QObject
{
    Q_OBJECT
public:
    explicit SyncStateDatabase(QObject* parent = nullptr);
    ~SyncStateDatabase() override;

    virtual bool openDatabase();
    virtual bool addEntry(const SyncStateEntry& entry) = 0;
    virtual SyncStateEntry getEntry(const QString& path) = 0;
    virtual QVector<SyncStateEntry> findEntries(const QString& parent, bool* ok = nullptr) = 0;
    virtual bool removeEntries(const QString& path) = 0;
    virtual bool removeEntry(const QString& path) = 0;
    virtual bool closeDatabase();

    bool isOpen() const;

    bool iterate(std::function<void(const SyncStateEntry& entry)> callback,
                 const QString& path = "/");

protected:
    explicit SyncStateDatabase(SyncStateDatabasePrivate* d, QObject* parent = nullptr);

    QScopedPointer<SyncStateDatabasePrivate> d_ptr;
    Q_DECLARE_PRIVATE(SyncStateDatabase);

    void setOpen(bool open);
};

} // namespace SynqClient

#endif // SYNQCLIENT_SYNCSTATEDATABASE_H
