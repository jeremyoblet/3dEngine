#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

class GizmoRotation {
public:
    GizmoRotation();
    ~GizmoRotation();

    void draw(const glm::mat4& view, const glm::mat4& proj,
              const glm::vec3& position, float scale);

private:
    unsigned int VAO, VBO;
    unsigned int shaderProgram;
    int          vertexCount;

    unsigned int compileShader(unsigned int type, const char* source);
};
