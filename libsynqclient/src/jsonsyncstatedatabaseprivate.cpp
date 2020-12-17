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

#include "jsonsyncstatedatabaseprivate.h"

#include <QLoggingCategory>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QVersionNumber>

namespace SynqClient {

static Q_LOGGING_CATEGORY(log, "SynqClient.JSONSyncStateDatabase", QtWarningMsg);

const char* JSONSyncStateDatabasePrivate::EntryProperty = "entry";
const char* JSONSyncStateDatabasePrivate::ChildrenProperty = "children";
const char* JSONSyncStateDatabasePrivate::ModificationTimeProperty = "modificationTime";
const char* JSONSyncStateDatabasePrivate::SyncPropertyProperty = "syncProperty";
const char* JSONSyncStateDatabasePrivate::VersionProperty = "version";

const char* JSONSyncStateDatabasePrivate::Version_1_0 = "1.0";
const char* JSONSyncStateDatabasePrivate::CurrentVersion =
        JSONSyncStateDatabasePrivate::Version_1_0;

JSONSyncStateDatabasePrivate::JSONSyncStateDatabasePrivate(JSONSyncStateDatabase* q)
    : SyncStateDatabasePrivate(q), filename(), data()
{
}

JSONSyncStateDatabasePrivate::Node*
JSONSyncStateDatabasePrivate::findNode(const SyncStateEntry& entry,
                                       JSONSyncStateDatabasePrivate::FindNodeMode mode)
{
    return findNode(entry.path(), mode);
}

JSONSyncStateDatabasePrivate::Node*
JSONSyncStateDatabasePrivate::findNode(const QString& path,
                                       JSONSyncStateDatabasePrivate::FindNodeMode mode)
{
    Node* result = &data;
    auto parts = path.split("/", Qt::SkipEmptyParts);
    for (const auto& part : parts) {
        if (result->children.contains(part)) {
            result = &result->children[part];
        } else {
            switch (mode) {
            case FindNodeMode::Find:
                result = nullptr;
                break;
            case FindNodeMode::FindAndCreate:
                result->children.insert(part, Node());
                result = &result->children[part];
                break;
            }
        }
        if (result == nullptr) {
            break;
        }
    }
    return result;
}

bool JSONSyncStateDatabasePrivate::jsonToNode(const QJsonObject& object,
                                              JSONSyncStateDatabasePrivate::Node& node)
{
    if (object.contains(EntryProperty)) {
        auto entryValue = object.value(EntryProperty);
        if (entryValue.isObject()) {
            auto entryData = entryValue.toObject();
            auto modificationTimeValue = entryData.value(ModificationTimeProperty);
            auto syncPropertyValue = entryData.value(SyncPropertyProperty);
            if (modificationTimeValue.isString() && syncPropertyValue.isString()) {
                SyncStateEntry entry;
                entry.setModificationTime(
                        QDateTime::fromString(modificationTimeValue.toString(), Qt::ISODateWithMs));
                entry.setSyncProperty(syncPropertyValue.toString());
                entry.setValid(true);
                node.entry = entry;
            } else {
                qCWarning(log) << "Entry data contains invalid data";
                return false;
            }
        } else {
            qCWarning(log) << "Entry is not a JSON object";
            return false;
        }
    }
    node.children.clear();
    if (object.contains(ChildrenProperty)) {
        auto childrenValue = object.value(ChildrenProperty);
        if (childrenValue.isObject()) {
            auto childrenData = childrenValue.toObject();
            for (const auto& name : childrenData.keys()) {
                auto value = childrenData.value(name);
                if (value.isObject()) {
                    Node newNode;
                    auto ok = jsonToNode(value.toObject(), newNode);
                    if (ok) {
                        node.children[name] = newNode;
                    } else {
                        qCWarning(log) << "Failed to load child" << name;
                        return false;
                    }
                } else {
                    qCWarning(log) << "Child data must be a JSON object";
                    return false;
                }
            }
        } else {
            qCWarning(log) << "Children data must be a JSON object";
            return false;
        }
    }
    return true;
}

QVariantMap JSONSyncStateDatabasePrivate::nodeToJson(const JSONSyncStateDatabasePrivate::Node& node)
{
    QVariantMap result;
    if (node.entry.isValid()) {
        QVariantMap entry { { ModificationTimeProperty,
                              node.entry.modificationTime().toString(Qt::ISODateWithMs) },
                            { SyncPropertyProperty, node.entry.syncProperty() } };
        result[EntryProperty] = entry;
    }
    if (!node.children.isEmpty()) {
        QVariantMap children;
        for (const auto& childName : node.children.keys()) {
            children[childName] = nodeToJson(node.children[childName]);
        }
        result[ChildrenProperty] = children;
    }
    return result;
}

bool JSONSyncStateDatabasePrivate::checkCanHandleVersion(const QJsonObject& object)
{
    if (object.contains(VersionProperty)) {
        auto version = object.value(VersionProperty);
        if (version.isString()) {
            auto fileVersion = QVersionNumber::fromString(version.toString());
            if (fileVersion != QVersionNumber::fromString(Version_1_0)) {
                qCWarning(log) << "Cannot handle JSON sync state database of version"
                               << version.toString();
                return false;
            }
        } else {
            qCWarning(log) << "Database version is not a string";
            return false;
        }
    } else {
        qCWarning(log) << "Database is missing version number";
        return false;
    }
    return true;
}

} // namespace SynqClient
