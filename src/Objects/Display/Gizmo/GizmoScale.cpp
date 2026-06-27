#include "GizmoScale.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>

// ---- Shaders ---------------------------------------------------------------

static const char* VERT_SRC = R"(
    #version 450 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec3 aColor;
    uniform mat4 uMVP;
    out vec3 vColor;
    void main() {
        gl_Position = uMVP * vec4(aPos, 1.0);
        vColor = aColor;
    }
)";

static const char* FRAG_SRC = R"(
    #version 450 core
    in vec3 vColor;
    out vec4 FragColor;
    void main() {
        FragColor = vec4(vColor, 1.0);
    }
)";

// ---- Helpers ---------------------------------------------------------------

static void pushFace(std::vector<float>& v,
                     glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d,
                     glm::vec3 col)
{
    // 2 triangles : abc + acd
    for (auto p : { a, b, c, a, c, d })
        v.insert(v.end(), { p.x, p.y, p.z, col.r, col.g, col.b });
}

// Génère les 6 faces d'un cube AABB [center - h, center + h]
static void pushCube(std::vector<float>& v, glm::vec3 center, float h, glm::vec3 col)
{
    float x0 = center.x - h, x1 = center.x + h;
    float y0 = center.y - h, y1 = center.y + h;
    float z0 = center.z - h, z1 = center.z + h;

    pushFace(v, {x0,y0,z1},{x1,y0,z1},{x1,y1,z1},{x0,y1,z1}, col); // +Z
    pushFace(v, {x1,y0,z0},{x0,y0,z0},{x0,y1,z0},{x1,y1,z0}, col); // -Z
    pushFace(v, {x0,y0,z0},{x1,y0,z0},{x1,y0,z1},{x0,y0,z1}, col); // -Y
    pushFace(v, {x0,y1,z1},{x1,y1,z1},{x1,y1,z0},{x0,y1,z0}, col); // +Y
    pushFace(v, {x0,y0,z0},{x0,y0,z1},{x0,y1,z1},{x0,y1,z0}, col); // -X
    pushFace(v, {x1,y0,z1},{x1,y0,z0},{x1,y1,z0},{x1,y1,z1}, col); // +X
}

// ---- Implémentation --------------------------------------------------------

unsigned int GizmoScale::compileShader(unsigned int type, const char* source)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    int ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "GizmoScale shader error: " << log << std::endl;
    }
    return shader;
}

GizmoScale::GizmoScale()
{
    // Shader
    unsigned int vert = compileShader(GL_VERTEX_SHADER,   VERT_SRC);
    unsigned int frag = compileShader(GL_FRAGMENT_SHADER, FRAG_SRC);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vert);
    glAttachShader(shaderProgram, frag);
    glLinkProgram(shaderProgram);
    glDeleteShader(vert);
    glDeleteShader(frag);

    constexpr int STRIDE = 6 * sizeof(float);

    // --- Tiges (de l'origine à 0.75 sur chaque axe) ---
    static constexpr float SHAFTS[] = {
        0.0f, 0.0f, 0.0f,  1.0f, 0.15f,0.15f,   // X rouge
        0.75f,0.0f, 0.0f,  1.0f, 0.15f,0.15f,
        0.0f, 0.0f, 0.0f,  0.15f,1.0f, 0.15f,   // Y vert
        0.0f, 0.75f,0.0f,  0.15f,1.0f, 0.15f,
        0.0f, 0.0f, 0.0f,  0.15f,0.15f,1.0f,    // Z bleu
        0.0f, 0.0f, 0.75f, 0.15f,0.15f,1.0f,
    };

    glGenVertexArrays(1, &shaftsVAO);
    glGenBuffers(1, &shaftsVBO);
    glBindVertexArray(shaftsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, shaftsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SHAFTS), SHAFTS, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // --- Cubes aux extrémités (centrés à 1.0 sur chaque axe) ---
    const float H = 0.065f; // demi-taille du cube

    std::vector<float> cubeVerts;
    pushCube(cubeVerts, {1.0f, 0.0f, 0.0f}, H, {1.0f, 0.15f,0.15f}); // X
    pushCube(cubeVerts, {0.0f, 1.0f, 0.0f}, H, {0.15f,1.0f, 0.15f}); // Y
    pushCube(cubeVerts, {0.0f, 0.0f, 1.0f}, H, {0.15f,0.15f,1.0f });  // Z

    cubesCount = static_cast<int>(cubeVerts.size()) / 6;

    glGenVertexArrays(1, &cubesVAO);
    glGenBuffers(1, &cubesVBO);
    glBindVertexArray(cubesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubesVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(cubeVerts.size() * sizeof(float)),
                 cubeVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

GizmoScale::~GizmoScale()
{
    glDeleteVertexArrays(1, &shaftsVAO);
    glDeleteBuffers(1, &shaftsVBO);
    glDeleteVertexArrays(1, &cubesVAO);
    glDeleteBuffers(1, &cubesVBO);
    glDeleteProgram(shaderProgram);
}

void GizmoScale::draw(const glm::mat4& view, const glm::mat4& proj,
                      const glm::vec3& position, float scale)
{
    glm::mat4 model = glm::scale(
        glm::translate(glm::mat4(1.0f), position),
        glm::vec3(scale));
    const glm::mat4 MVP = proj * view * model;

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uMVP"),
                       1, GL_FALSE, glm::value_ptr(MVP));

    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(shaftsVAO);
    glDrawArrays(GL_LINES, 0, 6);

    glBindVertexArray(cubesVAO);
    glDrawArrays(GL_TRIANGLES, 0, cubesCount);

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}
