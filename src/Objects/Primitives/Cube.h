#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "../Lights/PointLight.h"

class Cube {
public:
    Cube();
    ~Cube();

    void draw(const glm::mat4& MVP, const glm::mat4& model,
              const PointLight& light, const glm::vec3& viewPos);

private:
    unsigned int VAO, VBO;
    unsigned int shaderProgram;

    unsigned int compileShader(unsigned int type, const char* source);
};
