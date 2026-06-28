#pragma once
#include "Core/Object.h"
#include <glm/glm.hpp>

class PointLightObject : public Object {
public:
    glm::vec3 lightColor = { 1.0f, 1.0f, 1.0f };
    float     intensity  = 3.0f;

    PointLightObject();
};
