#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

class GizmoScale {
public:
    GizmoScale();
    ~GizmoScale();

    void draw(const glm::mat4& view, const glm::mat4& proj,
              const glm::vec3& position, float scale);

private:
    unsigned int shaftsVAO, shaftsVBO;   // tiges  (GL_LINES)
    unsigned int cubesVAO,  cubesVBO;    // cubes au bout (GL_TRIANGLES)
    unsigned int shaderProgram;
    int          cubesCount;

    unsigned int compileShader(unsigned int type, const char* source);
};
