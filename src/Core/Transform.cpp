#include "Transform.h"
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 Transform::getMatrix() const
{
    glm::mat4 T = glm::translate(glm::mat4(1.0f), position);

    glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), { 1, 0, 0 });
    R           = glm::rotate(R,                glm::radians(rotation.y), { 0, 1, 0 });
    R           = glm::rotate(R,                glm::radians(rotation.z), { 0, 0, 1 });

    glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

    return T * R * S;
}
