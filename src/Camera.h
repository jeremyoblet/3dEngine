#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    Camera(float distance = 3.0f);

    // Matrices
    glm::mat4 getView()       const;
    glm::mat4 getProjection(float aspectRatio) const;

    // Inputs
    void orbit(float dx, float dy);
    void pan(float dx, float dy);
    void zoom(float delta);

private:
    glm::vec3 target;
    float distance;
    float yaw;    // rotation horizontale (degrés)
    float pitch;  // rotation verticale   (degrés)

    glm::vec3 getPosition() const;
};