#include "Scene.h"

SceneNode* Scene::addObject(const std::string& name, std::unique_ptr<Object> object, SceneNode* parent)
{
    auto node    = std::make_unique<SceneNode>();
    node->name   = name;
    node->object = std::move(object);
    node->parent = parent;

    SceneNode* ptr = node.get();

    if (parent)
        parent->children.push_back(std::move(node));
    else
        m_roots.push_back(std::move(node));

    return ptr;
}

void Scene::draw(const DrawContext& ctx) const
{
    for (const auto& root : m_roots)
        drawNode(*root, ctx);
}

void Scene::drawNode(const SceneNode& node, const DrawContext& ctx) const
{
    if (node.object)
        node.object->draw(ctx);
    for (const auto& child : node.children)
        drawNode(*child, ctx);
}

const std::vector<std::unique_ptr<SceneNode>>& Scene::roots() const
{
    return m_roots;
}
