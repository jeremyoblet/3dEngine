#include "SceneModel.h"

SceneModel::SceneModel(const Scene& scene, QObject* parent)
    : QAbstractItemModel(parent), m_scene(scene)
{}

QModelIndex SceneModel::index(int row, int col, const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        const auto& roots = m_scene.roots();
        if (row < (int)roots.size())
            return createIndex(row, col, roots[row].get());
    } else {
        auto* node = static_cast<SceneNode*>(parent.internalPointer());
        if (row < (int)node->children.size())
            return createIndex(row, col, node->children[row].get());
    }
    return {};
}

QModelIndex SceneModel::parent(const QModelIndex& index) const
{
    if (!index.isValid()) return {};

    auto* node = static_cast<SceneNode*>(index.internalPointer());
    if (!node->parent) return {};

    SceneNode* grandparent      = node->parent->parent;
    const auto& siblings        = grandparent ? grandparent->children : m_scene.roots();

    for (int i = 0; i < (int)siblings.size(); ++i)
        if (siblings[i].get() == node->parent)
            return createIndex(i, 0, node->parent);

    return {};
}

int SceneModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid())
        return (int)m_scene.roots().size();
    auto* node = static_cast<SceneNode*>(parent.internalPointer());
    return (int)node->children.size();
}

int SceneModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant SceneModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole) return {};
    auto* node = static_cast<SceneNode*>(index.internalPointer());
    return QString::fromStdString(node->name);
}

void SceneModel::refresh()
{
    beginResetModel();
    endResetModel();
}

QModelIndex SceneModel::indexForNode(SceneNode* node) const
{
    if (!node) return {};
    const auto& siblings = node->parent ? node->parent->children : m_scene.roots();
    for (int i = 0; i < (int)siblings.size(); ++i)
        if (siblings[i].get() == node)
            return createIndex(i, 0, node);
    return {};
}
