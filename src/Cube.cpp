#include "Cube.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// ---- Shaders ---------------------------------------------------------------

static const char* VERT_SRC = R"(
    #version 450 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec3 aColor;
    uniform mat4 MVP;
    out vec3 vColor;
    void main() {
        gl_Position = MVP * vec4(aPos, 1.0);
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

// ---- Géométrie : 6 faces × 2 triangles × 3 sommets = 36 sommets ----------
// Chaque sommet : position (3 floats) + couleur (3 floats)

static constexpr float VERTICES[] = {
    // Face avant  (z = +0.5)  rouge
    -0.5f, -0.5f,  0.5f,  1.0f, 0.2f, 0.2f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.2f, 0.2f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.2f, 0.2f,
    -0.5f, -0.5f,  0.5f,  1.0f, 0.2f, 0.2f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.2f, 0.2f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.2f, 0.2f,

    // Face arrière (z = -0.5)  vert
     0.5f, -0.5f, -0.5f,  0.2f, 1.0f, 0.2f,
    -0.5f, -0.5f, -0.5f,  0.2f, 1.0f, 0.2f,
    -0.5f,  0.5f, -0.5f,  0.2f, 1.0f, 0.2f,
     0.5f, -0.5f, -0.5f,  0.2f, 1.0f, 0.2f,
    -0.5f,  0.5f, -0.5f,  0.2f, 1.0f, 0.2f,
     0.5f,  0.5f, -0.5f,  0.2f, 1.0f, 0.2f,

    // Face gauche (x = -0.5)  bleu
    -0.5f, -0.5f, -0.5f,  0.2f, 0.2f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.2f, 0.2f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.2f, 0.2f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.2f, 0.2f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.2f, 0.2f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.2f, 0.2f, 1.0f,

    // Face droite (x = +0.5)  jaune
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.2f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.2f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.2f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.2f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.2f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.2f,

    // Face haut   (y = +0.5)  cyan
    -0.5f,  0.5f,  0.5f,  0.2f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.2f, 1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.2f, 1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.2f, 1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.2f, 1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.2f, 1.0f, 1.0f,

    // Face bas    (y = -0.5)  magenta
    -0.5f, -0.5f, -0.5f,  1.0f, 0.2f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.2f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.2f, 1.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 0.2f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.2f, 1.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 0.2f, 1.0f,
};

// ---- Implémentation --------------------------------------------------------

unsigned int Cube::compileShader(unsigned int type, const char* source)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == 0)
    {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader error: " << log << std::endl;
    }
    return shader;
}

Cube::Cube()
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

    // Géométrie
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);

    constexpr int STRIDE = 6 * sizeof(float);

    // location 0 : position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)0);
    glEnableVertexAttribArray(0);

    // location 1 : couleur
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

Cube::~Cube()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
}

void Cube::draw(const glm::mat4& MVP)
{
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "MVP"),
        1, GL_FALSE, glm::value_ptr(MVP));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
