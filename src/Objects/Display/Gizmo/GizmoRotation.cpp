#include "GizmoRotation.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cmath>
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

// ---- Implémentation --------------------------------------------------------

unsigned int GizmoRotation::compileShader(unsigned int type, const char* source)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    int ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "GizmoRotation shader error: " << log << std::endl;
    }
    return shader;
}

GizmoRotation::GizmoRotation()
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

    // Géométrie : 3 anneaux × N segments × 2 sommets, chacun pos(3)+color(3)
    const int   N      = 64;
    const float TWO_PI = 6.28318530718f;

    struct Ring { glm::vec3 u, v, color; };
    const Ring rings[3] = {
        { {0,1,0}, {0,0,1}, {1.0f, 0.15f, 0.15f} }, // anneau X (plan YZ)
        { {1,0,0}, {0,0,1}, {0.15f,1.0f,  0.15f} }, // anneau Y (plan XZ)
        { {1,0,0}, {0,1,0}, {0.15f,0.15f, 1.0f } }, // anneau Z (plan XY)
    };

    std::vector<float> verts;
    auto push = [&](glm::vec3 p, glm::vec3 c) {
        verts.insert(verts.end(), { p.x, p.y, p.z, c.r, c.g, c.b });
    };

    for (const auto& ring : rings) {
        for (int i = 0; i < N; ++i) {
            float a0 = TWO_PI * i       / N;
            float a1 = TWO_PI * (i + 1) / N;
            push(std::cos(a0) * ring.u + std::sin(a0) * ring.v, ring.color);
            push(std::cos(a1) * ring.u + std::sin(a1) * ring.v, ring.color);
        }
    }

    vertexCount = static_cast<int>(verts.size()) / 6;

    constexpr int STRIDE = 6 * sizeof(float);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(verts.size() * sizeof(float)),
                 verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

GizmoRotation::~GizmoRotation()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
}

void GizmoRotation::draw(const glm::mat4& view, const glm::mat4& proj,
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
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, vertexCount);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}
