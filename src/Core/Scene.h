#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Object.h"
#include "DrawContext.h"

struct SceneNode {
    std::string                             name;
    std::unique_ptr<Object>                 object;
    SceneNode*                              parent   = nullptr;
    std::vector<std::unique_ptr<SceneNode>> children;
};

class Scene {
public:
    float unitSize = 1.0f;

    SceneNode* addObject(const std::string& name, std::unique_ptr<Object> object, SceneNode* parent = nullptr);
    void       draw(const DrawContext& ctx) const;

    const std::vector<std::unique_ptr<SceneNode>>& roots() const;

private:
    std::vector<std::unique_ptr<SceneNode>> m_roots;

    void drawNode(const SceneNode& node, const DrawContext& ctx) const;
};
