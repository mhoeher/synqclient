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

#include "../inc/jsonsyncstatedatabase.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QSaveFile>

#include "jsonsyncstatedatabaseprivate.h"

namespace SynqClient {

static Q_LOGGING_CATEGORY(log, "SynqClient.JSONSyncStateDatabase", QtWarningMsg);

/**
 * @class JSONSyncStateDatabase
 * @brief Store persistent sync state information in a single JSON file.
 *
 * This class can be used to store sync state information in a JSON file on disk. In order to
 * be usable, a file must be set from which to read and into which to write to state information.
 */

/**
 * @brief Constructor.
 *
 * Creates a new JSON sync state database which saves its data to the given
 * @p filename.
 */
JSONSyncStateDatabase::JSONSyncStateDatabase(const QString& filename, QObject* parent)
    : SyncStateDatabase(new JSONSyncStateDatabasePrivate(this), parent)
{
    Q_D(JSONSyncStateDatabase);
    d->filename = filename;
}

/**
 * @brief Constructor.
 *
 * Creates an empty JSON sync state database. Use setFilename() to set the
 * path to the file where to save to and load from persistent information.
 */
JSONSyncStateDatabase::JSONSyncStateDatabase(QObject* parent)
    : SyncStateDatabase(new JSONSyncStateDatabasePrivate(this), parent)
{
}

/**
 * @brief Destructor.
 */
JSONSyncStateDatabase::~JSONSyncStateDatabase() {}

/**
 * @brief The path to the file used to hold persistent information.
 */
QString JSONSyncStateDatabase::filename() const
{
    Q_D(const JSONSyncStateDatabase);
    return d->filename;
}

/**
 * @brief Set the path to the file where to store persistent information.
 */
void JSONSyncStateDatabase::setFilename(const QString& filename)
{
    Q_D(JSONSyncStateDatabase);
    d->filename = filename;
}

/**
 * @brief Constructor.
 */
JSONSyncStateDatabase::JSONSyncStateDatabase(JSONSyncStateDatabasePrivate* d, QObject* parent)
    : SyncStateDatabase(d, parent)
{
}

/**
 * @brief Open the sync state database.
 *
 * This will try to open the JSON file referred to by the filename() property.
 * If the file does not exist, it will be created and opened. If opening or creating the file fails,
 * this method returns false.
 */
bool JSONSyncStateDatabase::openDatabase()
{
    Q_D(JSONSyncStateDatabase);
    if (isOpen()) {
        qCWarning(log) << "JSON sync state database is already open";
        return false;
    }
    if (!d->filename.isEmpty()) {
        QFile file(d->filename);
        if (!file.exists()) {
            if (file.open(QIODevice::WriteOnly)) {
                // File did not yet exist but could be created - fine
                d->data.clear();
                file.close();
                setOpen(true);
                return true;
            } else {
                // File did not yet exist - error creating it!
                qCWarning(log) << "Failed to create JSON sync state database:"
                               << file.errorString();
            }
        } else {
            if (file.open(QIODevice::ReadOnly)) {
                QJsonParseError error;
                auto content = file.readAll();
                auto doc = QJsonDocument::fromJson(content);
                if (error.error == QJsonParseError::NoError) {
                    if (doc.isObject()) {
                        file.close();
                        auto docObj = doc.object();
                        if (!d->checkCanHandleVersion(docObj)) {
                            return false;
                        }
                        auto ok = d->jsonToNode(doc.object(), d->data);
                        if (ok) {
                            setOpen(true);
                            return true;
                        } else {
                            qCWarning(log) << "JSON sync state database is invalid";
                            d->data.clear();
                            return false;
                        }
                    } else {
                        qCWarning(log) << "JSON sync state database must be a JSON object";
                    }
                } else {
                    qCWarning(log)
                            << "Failed to parse JSON sync state database:" << error.errorString();
                }
            } else {
                qCWarning(log) << "Failed to open JSON sync state database for reading:"
                               << file.errorString();
            }
        }
    } else {
        qCWarning(log) << "No JSON sync state database filename set";
    }
    return false;
}

bool JSONSyncStateDatabase::closeDatabase()
{
    Q_D(JSONSyncStateDatabase);
    if (!isOpen()) {
        qCWarning(log) << "JSON sync state database is not open";
        return false;
    }
    setOpen(false);
    if (d->filename.isEmpty()) {
        qCWarning(log) << "No JSON sync state database filename set";
        return false;
    }

    QVariantMap data = d->nodeToJson(d->data);
    data[JSONSyncStateDatabasePrivate::VersionProperty] =
            JSONSyncStateDatabasePrivate::CurrentVersion;
    d->data.clear();

    QSaveFile file(d->filename);
    if (file.open(QIODevice::WriteOnly)) {
        auto doc = QJsonDocument::fromVariant(data);
        auto json = doc.toJson(QJsonDocument::Compact);
        file.write(json);
        if (file.commit()) {
            return true;
        } else {
            qCWarning(log) << "Failed to commit changes to JSON sync state database:"
                           << file.errorString();
        }
    } else {
        qCWarning(log) << "Failed to open JSON sync state database for writing:"
                       << file.errorString();
    }
    return false;
}

bool JSONSyncStateDatabase::addEntry(const SyncStateEntry& entry)
{
    Q_D(JSONSyncStateDatabase);
    if (!entry.isValid()) {
        return false;
    }
    auto node = d->findNode(entry, JSONSyncStateDatabasePrivate::FindNodeMode::FindAndCreate);
    node->entry = entry;
    return true;
}

SyncStateEntry JSONSyncStateDatabase::getEntry(const QString& path)
{
    Q_D(JSONSyncStateDatabase);
    SyncStateEntry result;
    if (path.isEmpty()) {
        return result;
    }

    auto node = d->findNode(path);
    if (node != nullptr) {
        result = node->entry;
        result.setPath(path);
    }
    return result;
}

QVector<SyncStateEntry> JSONSyncStateDatabase::findEntries(const QString& parent, bool* ok)
{
    Q_D(JSONSyncStateDatabase);
    QVector<SyncStateEntry> result;
    auto node = d->findNode(parent);
    QDir dir(parent);
    if (node) {
        for (const auto& childName : node->children.keys()) {
            const auto& child = node->children.value(childName);
            if (child.entry.isValid()) {
                SyncStateEntry entry;
                entry.setPath(dir.absoluteFilePath(childName));
                entry.setModificationTime(child.entry.modificationTime());
                entry.setSyncProperty(child.entry.syncProperty());
                entry.setValid(true);
                result << entry;
            }
        }
    }
    if (ok) {
        *ok = true;
    }
    return result;
}

bool JSONSyncStateDatabase::removeEntries(const QString& path)
{
    Q_D(JSONSyncStateDatabase);
    auto node = d->findNode(path);
    if (node) {
        node->children.clear();
        node->entry = SyncStateEntry();
    }
    return true;
}

bool JSONSyncStateDatabase::removeEntry(const QString& path)
{
    Q_D(JSONSyncStateDatabase);
    auto node = d->findNode(path);
    if (node) {
        node->entry.setValid(false);
    }
    return true;
}

} // namespace SynqClient
