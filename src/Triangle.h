#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

class Triangle {
public:
    Triangle();
    ~Triangle();

    void draw(const glm::mat4& MVP);

private:
    unsigned int VAO, VBO;
    unsigned int shaderProgram;

    static constexpr float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };

    unsigned int compileShader(unsigned int type, const char* source);
};