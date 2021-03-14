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

#ifndef SYNQCLIENT_JSONSYNCSTATEDATABASE_H
#define SYNQCLIENT_JSONSYNCSTATEDATABASE_H

#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>

#include "SyncStateDatabase"
#include "libsynqclient_global.h"

namespace SynqClient {

class JSONSyncStateDatabasePrivate;

class LIBSYNQCLIENT_EXPORT JSONSyncStateDatabase : public SyncStateDatabase
{
    Q_OBJECT
public:
    explicit JSONSyncStateDatabase(const QString& filename, QObject* parent = nullptr);
    explicit JSONSyncStateDatabase(QObject* parent = nullptr);
    ~JSONSyncStateDatabase() override;

    QString filename() const;
    void setFilename(const QString& filename);

protected:
    explicit JSONSyncStateDatabase(JSONSyncStateDatabasePrivate* d, QObject* parent = nullptr);

    Q_DECLARE_PRIVATE(JSONSyncStateDatabase);

    // SyncStateDatabase interface
public:
    bool openDatabase() override;
    bool closeDatabase() override;
    bool addEntry(const SyncStateEntry& entry) override;
    SyncStateEntry getEntry(const QString& path) override;
    QVector<SyncStateEntry> findEntries(const QString& parent, bool* ok) override;
    bool removeEntries(const QString& path) override;
    bool removeEntry(const QString& path) override;
};

} // namespace SynqClient

#endif // SYNQCLIENT_JSONSYNCSTATEDATABASE_H
