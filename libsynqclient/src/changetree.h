#ifndef SYNQCLIENT_CHANGETREE_H
#define SYNQCLIENT_CHANGETREE_H

#include <iostream>

#include <QDateTime>
#include <QMap>
#include <QQueue>
#include <QSet>
#include <QString>

#include "SynqClient/SyncStateEntry"

// Uncomment to enable debug output of the change trees to stderr. Useful during writing code/unit
// tests.
//#define SYNQCLIENT_ENABLE_CHANGETREE_DUMP

namespace SynqClient {

struct ChangeTreeNode;

struct ChangeTree
{
    enum FindMode { Find, FindAndCreate };
    enum ChangeType { Unknown, Created, Changed, Deleted };
    enum NodeType { Invalid, Folder, File };

    ChangeTree();
    ChangeTree(const ChangeTree& other);
    ~ChangeTree();
    ChangeTree& operator=(const ChangeTree& other);

    ChangeTreeNode* root;

    ChangeTreeNode* findNode(const QString& path, FindMode mode = Find);
    static const ChangeTreeNode* findNode(const ChangeTreeNode& node,
                                          std::function<bool(const ChangeTreeNode& node)> filter);

    template<ChangeType changeType>
    static bool has(const ChangeTreeNode& node);

    static bool hasAnyChange(const ChangeTreeNode& node);

    static QSet<QString> mergeNames(const ChangeTreeNode* first, const ChangeTreeNode* second,
                                    const QString& prefix = "");

    void dump(const QString& text);
    void normalize();
};

/**
 * @brief Represents a node in a tree used to represent all changes.
 */
struct ChangeTreeNode
{

    typedef QMap<QString, ChangeTreeNode> Children;

    ChangeTree::NodeType type = ChangeTree::Invalid;
    ChangeTree::ChangeType change = ChangeTree::Unknown;
    QDateTime lastModified = QDateTime();
    QString syncAttribute = QString();
    Children children = Children();

    void dump(const QString& name, const QString& indentation = "") const;
    void normalize();
};

inline ChangeTree::ChangeTree() : root(new ChangeTreeNode)
{
    root->type = ChangeTree::Folder;
}

inline ChangeTree::ChangeTree(const ChangeTree& other) : root(new ChangeTreeNode)
{
    *root = *(other.root);
}

inline ChangeTree::~ChangeTree()
{
    delete root;
}

inline ChangeTree& ChangeTree::operator=(const ChangeTree& other)
{
    *root = *(other.root);
    return *this;
}

inline ChangeTreeNode* ChangeTree::findNode(const QString& path, ChangeTree::FindMode mode)
{
    auto result = root;
    auto parts = SyncStateEntry::makePath("/" + path).split("/", Qt::SkipEmptyParts);
    while (!parts.isEmpty() && result != nullptr) {
        auto part = parts.takeFirst();
        switch (mode) {
        case Find:
            if (result->children.contains(part)) {
                result = &result->children[part];
            } else {
                result = nullptr;
            }
            break;
        case FindAndCreate: {
            auto& child = result->children[part];
            result = &child;
            if (!parts.isEmpty()) {
                child.type = ChangeTree::Folder;
            }
            break;
        }
        }
    }
    return result;
}

/**
 * @brief Find a node using a filter function.
 *
 * This method traverses the sub-tree starting with the given @p node. The pointer to the first node
 * for which @p filter returns true is returned.
 *
 * If the filter does not return true for any node, a nullptr is returned.
 */
inline const ChangeTreeNode* ChangeTree::findNode(const ChangeTreeNode& node,
                                                  std::function<bool(const ChangeTreeNode&)> filter)
{
    QQueue<const ChangeTreeNode*> queue;
    queue << &node;
    while (!queue.isEmpty()) {
        const auto n = queue.dequeue();
        if (filter(*n)) {
            return n;
        }
        for (auto& child : n->children) {
            queue << &child;
        }
    }
    return nullptr;
}

inline bool ChangeTree::hasAnyChange(const ChangeTreeNode& node)
{
    return findNode(node, [=](const ChangeTreeNode& n) { return n.change != ChangeTree::Unknown; })
            != nullptr;
}

inline QSet<QString> ChangeTree::mergeNames(const ChangeTreeNode* first,
                                            const ChangeTreeNode* second, const QString& prefix)
{
    QSet<QString> result;
    if (first != nullptr) {
        for (auto it = first->children.cbegin(); it != first->children.cend(); ++it) {
            result.insert(prefix + it.key());
        }
    }
    if (second != nullptr) {
        for (auto it = second->children.cbegin(); it != second->children.cend(); ++it) {
            result.insert(prefix + it.key());
        }
    }
    return result;
}

inline void ChangeTree::dump(const QString& text)
{
#ifdef SYNQCLIENT_ENABLE_CHANGETREE_DUMP
    std::cerr << qUtf8Printable(text) << std::endl;
    for (auto it = root->children.cbegin(); it != root->children.cend(); ++it) {
        it.value().dump(it.key());
    }
#else
    Q_UNUSED(text);
#endif
}

/**
 * @brief Normalizes the change tree.
 *
 * This runs some normalizations on the tree. In particular:
 *
 * - Do not mark a node as deleted, if some child nodes have changes.
 */
inline void ChangeTree::normalize()
{
    root->normalize();
}

template<ChangeTree::ChangeType changeType>
bool ChangeTree::has(const ChangeTreeNode& node)
{
    return findNode(node, [=](const ChangeTreeNode& node) { return node.change == changeType; })
            != nullptr;
}

inline void ChangeTreeNode::dump(const QString& name, const QString& indentation) const
{
#ifdef SYNQCLIENT_ENABLE_CHANGETREE_DUMP
    QMap<int, const char*> typeNames { { ChangeTree::Invalid, " " },
                                       { ChangeTree::Folder, "D" },
                                       { ChangeTree::File, "F" } };
    QMap<int, const char*> changeNames { { ChangeTree::Unknown, "?" },
                                         { ChangeTree::Created, "N" },
                                         { ChangeTree::Changed, "U" },
                                         { ChangeTree::Deleted, "D" } };
    std::cerr << qUtf8Printable(indentation) << typeNames[type] << " " << changeNames[change] << " "
              << qUtf8Printable(name) << " " << qUtf8Printable(lastModified.toString()) << " "
              << qUtf8Printable(syncAttribute) << std::endl;
    for (auto it = children.cbegin(); it != children.cend(); ++it) {
        it.value().dump(it.key(), indentation + "    ");
    }
#else
    Q_UNUSED(name);
    Q_UNUSED(indentation);
#endif
}

inline void ChangeTreeNode::normalize()
{
    bool hasChildChanges = false;

    // First, normalize children:
    for (auto& child : children) {
        child.normalize();
        switch (child.change) {
        case ChangeTree::Changed:
        case ChangeTree::Created:
            hasChildChanges = true;
            break;
        case ChangeTree::Unknown:
        case ChangeTree::Deleted:
            break;
        }
    }

    // Correct own state:
    if (hasChildChanges) {
        switch (change) {
        case ChangeTree::Deleted:
        case ChangeTree::Unknown:
            change = ChangeTree::Changed;
            break;
        case ChangeTree::Created:
        case ChangeTree::Changed:
            break;
        }
    }
}

} // namespace SynqClient

#endif // SYNQCLIENT_CHANGETREE_H
