#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

Camera::Camera(float distance)
    : target(0.0f), distance(distance), yaw(45.0f), pitch(30.0f) {}

glm::vec3 Camera::getPosition() const {
    float yR = glm::radians(yaw);
    float pR = glm::radians(pitch);
    return target + distance * glm::vec3(
        cos(pR) * sin(yR),
        sin(pR),
        cos(pR) * cos(yR)
    );
}

glm::mat4 Camera::getView() const {
    return glm::lookAt(getPosition(), target, glm::vec3(0, 1, 0));
}

glm::mat4 Camera::getProjection(float aspectRatio) const {
    return glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
}

void Camera::orbit(float dx, float dy) {
    yaw   -= dx * 0.4f;
    pitch  = std::clamp(pitch + dy * 0.4f, -89.0f, 89.0f);
}

void Camera::pan(float dx, float dy) {
    // Calcule les axes right/up dans l'espace caméra
    glm::vec3 forward = glm::normalize(target - getPosition());
    glm::vec3 right   = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
    glm::vec3 up      = glm::cross(right, forward);

    target -= right * dx * distance * 0.001f;
    target += up    * dy * distance * 0.001f;
}

void Camera::zoom(float delta) {
    distance = std::clamp(distance - delta * 0.3f, 0.5f, 50.0f);
}