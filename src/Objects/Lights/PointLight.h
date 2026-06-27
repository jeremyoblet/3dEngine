#pragma once
#include <glm/glm.hpp>

struct PointLight {
    glm::vec3 position  = { 2.0f, 3.0f, 2.0f };
    glm::vec3 color     = { 1.0f, 1.0f, 1.0f };
    float     intensity = 3.0f;
};
