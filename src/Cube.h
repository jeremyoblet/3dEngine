#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

class Cube {
public:
    Cube();
    ~Cube();

    void draw(const glm::mat4& MVP);

private:
    unsigned int VAO, VBO;
    unsigned int shaderProgram;

    unsigned int compileShader(unsigned int type, const char* source);
};
