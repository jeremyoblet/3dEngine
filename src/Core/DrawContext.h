#pragma once
#include <glm/glm.hpp>
#include "Objects/Lights/PointLight.h"

struct DrawContext {
    glm::mat4  view;
    glm::mat4  proj;
    glm::vec3  viewPos;
    PointLight light;
};
