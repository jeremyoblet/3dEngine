#include "Grid.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

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

unsigned int Grid::compileShader(unsigned int type, const char* source)
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

Grid::Grid(int halfSize)
{
    // Couleurs
    const glm::vec3 lineColor  = { 0.30f, 0.30f, 0.30f };
    const glm::vec3 xAxisColor = { 0.65f, 0.15f, 0.15f }; // rouge  : axe X
    const glm::vec3 zAxisColor = { 0.15f, 0.15f, 0.65f }; // bleu   : axe Z

    // position (3) + couleur (3)
    std::vector<float> verts;

    auto push = [&](float x, float y, float z, const glm::vec3& c) {
        verts.insert(verts.end(), { x, y, z, c.r, c.g, c.b });
    };

    const float limit = static_cast<float>(halfSize);

    // Lignes parallèles à X (varient en Z)
    for (int i = -halfSize; i <= halfSize; ++i)
    {
        const float z = static_cast<float>(i);
        const glm::vec3& col = (i == 0) ? xAxisColor : lineColor;
        push(-limit, 0.0f, z, col);
        push( limit, 0.0f, z, col);
    }

    // Lignes parallèles à Z (varient en X)
    for (int i = -halfSize; i <= halfSize; ++i)
    {
        const float x = static_cast<float>(i);
        const glm::vec3& col = (i == 0) ? zAxisColor : lineColor;
        push(x, 0.0f, -limit, col);
        push(x, 0.0f,  limit, col);
    }

    vertexCount = static_cast<int>(verts.size()) / 6;

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
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(verts.size() * sizeof(float)),
                 verts.data(), GL_STATIC_DRAW);

    constexpr int STRIDE = 6 * sizeof(float);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

Grid::~Grid()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
}

void Grid::draw(const glm::mat4& MVP)
{
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uMVP"),
                       1, GL_FALSE, glm::value_ptr(MVP));

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, vertexCount);
    glBindVertexArray(0);
}
