#pragma once
#include <glm/glm.hpp>

class Transform {
public:
    glm::vec3 position = { 0.0f, 0.0f, 0.0f };
    glm::vec3 rotation = { 0.0f, 0.0f, 0.0f }; // degrés, ordre intrinsèque X→Y→Z
    glm::vec3 scale    = { 1.0f, 1.0f, 1.0f };

    // Retourne la matrice modèle TRS (Translate * Rotate * Scale)
    glm::mat4 getMatrix() const;
};
