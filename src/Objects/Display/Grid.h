#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

class Grid {
public:
    explicit Grid(int halfSize = 5);
    ~Grid();

    void draw(const glm::mat4& MVP);

private:
    unsigned int VAO, VBO;
    unsigned int shaderProgram;
    int          vertexCount;

    unsigned int compileShader(unsigned int type, const char* source);
};
