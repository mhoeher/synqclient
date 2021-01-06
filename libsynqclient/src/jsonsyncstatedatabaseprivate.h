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

#ifndef SYNQCLIENT_JSONSYNCSTATEDATABASEPRIVATE_H
#define SYNQCLIENT_JSONSYNCSTATEDATABASEPRIVATE_H

#include <QVariantMap>

#include "syncstatedatabaseprivate.h"
#include "jsonsyncstatedatabase.h"

class QJsonObject;

namespace SynqClient {

class JSONSyncStateDatabasePrivate : public SyncStateDatabasePrivate
{
public:
    enum class FindNodeMode { Find, FindAndCreate };

    struct Node
    {
        SyncStateEntry entry;
        QMap<QString, Node> children;

        void clear()
        {
            entry = SyncStateEntry();
            children.clear();
        }
    };

    static const char* EntryProperty;
    static const char* ChildrenProperty;
    static const char* ModificationTimeProperty;
    static const char* SyncPropertyProperty;
    static const char* VersionProperty;

    static const char* Version_1_0;
    static const char* CurrentVersion;

    explicit JSONSyncStateDatabasePrivate(JSONSyncStateDatabase* q);

    Q_DECLARE_PUBLIC(JSONSyncStateDatabase);

    QString filename;
    Node data;

    Node* findNode(const SyncStateEntry& entry, FindNodeMode mode = FindNodeMode::Find);
    Node* findNode(const QString& path, FindNodeMode mode = FindNodeMode::Find);

    bool jsonToNode(const QJsonObject& object, Node& node);
    QVariantMap nodeToJson(const Node& node);
    bool checkCanHandleVersion(const QJsonObject& object);
};

} // namespace SynqClient

#endif // SYNQCLIENT_JSONSYNCSTATEDATABASEPRIVATE_H
