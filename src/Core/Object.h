#pragma once
#include <memory>
#include "Transform.h"
#include "Shape.h"
#include "Material.h"
#include "DrawContext.h"

class Object {
public:
    Transform transform;

    Object(std::unique_ptr<Shape> shape, std::unique_ptr<Material> material);
    virtual ~Object() = default;

    virtual void draw(const DrawContext& ctx);

protected:
    std::unique_ptr<Shape>    shape;
    std::unique_ptr<Material> material;
};
