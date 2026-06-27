#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

class GizmoTranslation {
public:
    GizmoTranslation();
    ~GizmoTranslation();

    // Rendu par-dessus la scène (depth test désactivé).
    // scale : facteur visuel, mis à jour depuis Viewer chaque frame.
    void draw(const glm::mat4& view, const glm::mat4& proj,
              const glm::vec3& position, float scale);

private:
    unsigned int shaftsVAO, shaftsVBO;   // tiges  (GL_LINES)
    unsigned int headsVAO,  headsVBO;    // cônes  (GL_TRIANGLES)
    unsigned int shaderProgram;
    int          headsCount;

    unsigned int compileShader(unsigned int type, const char* source);
};
