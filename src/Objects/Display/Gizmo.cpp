#include "Gizmo.h"
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

// ---- Tiges (de l'origine à 0.75 sur chaque axe) ---------------------------

static constexpr float SHAFTS[] = {
    0.0f, 0.0f, 0.0f,  1.0f, 0.15f, 0.15f,   // X rouge
    0.75f,0.0f, 0.0f,  1.0f, 0.15f, 0.15f,
    0.0f, 0.0f, 0.0f,  0.15f,1.0f,  0.15f,   // Y vert
    0.0f, 0.75f,0.0f,  0.15f,1.0f,  0.15f,
    0.0f, 0.0f, 0.0f,  0.15f,0.15f, 1.0f,    // Z bleu
    0.0f, 0.0f, 0.75f, 0.15f,0.15f, 1.0f,
};

// ---- Implémentation --------------------------------------------------------

unsigned int Gizmo::compileShader(unsigned int type, const char* source)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    int ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Gizmo shader error: " << log << std::endl;
    }
    return shader;
}

Gizmo::Gizmo()
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

    // --- Tiges ---
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

    // --- Cônes (pointes des flèches) ---
    struct AxisDef { glm::vec3 dir, u, v, color; };
    const AxisDef axes[3] = {
        { {1,0,0}, {0,1,0}, {0,0,1}, {1.0f, 0.15f, 0.15f} },
        { {0,1,0}, {1,0,0}, {0,0,1}, {0.15f,1.0f,  0.15f} },
        { {0,0,1}, {1,0,0}, {0,1,0}, {0.15f,0.15f, 1.0f } },
    };

    const int   N         = 12;
    const float BASE      = 0.75f;
    const float CONE_R    = 0.07f;
    const float TWO_PI    = 6.28318530718f;

    std::vector<float> coneVerts;
    auto push = [&](glm::vec3 p, glm::vec3 c) {
        coneVerts.insert(coneVerts.end(), { p.x, p.y, p.z, c.r, c.g, c.b });
    };

    for (const auto& ax : axes) {
        glm::vec3 tip    = ax.dir * 1.0f;
        glm::vec3 center = ax.dir * BASE;

        for (int i = 0; i < N; ++i) {
            float a0 = TWO_PI * i       / N;
            float a1 = TWO_PI * (i + 1) / N;
            glm::vec3 p0 = center + CONE_R * (std::cos(a0) * ax.u + std::sin(a0) * ax.v);
            glm::vec3 p1 = center + CONE_R * (std::cos(a1) * ax.u + std::sin(a1) * ax.v);

            // Face latérale
            push(tip, ax.color);
            push(p0,  ax.color);
            push(p1,  ax.color);

            // Fond du cône
            push(center, ax.color);
            push(p1,     ax.color);
            push(p0,     ax.color);
        }
    }

    headsCount = static_cast<int>(coneVerts.size()) / 6;

    glGenVertexArrays(1, &headsVAO);
    glGenBuffers(1, &headsVBO);
    glBindVertexArray(headsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, headsVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(coneVerts.size() * sizeof(float)),
                 coneVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

Gizmo::~Gizmo()
{
    glDeleteVertexArrays(1, &shaftsVAO);
    glDeleteBuffers(1, &shaftsVBO);
    glDeleteVertexArrays(1, &headsVAO);
    glDeleteBuffers(1, &headsVBO);
    glDeleteProgram(shaderProgram);
}

void Gizmo::draw(const glm::mat4& view, const glm::mat4& proj,
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

    glBindVertexArray(headsVAO);
    glDrawArrays(GL_TRIANGLES, 0, headsCount);

    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}
