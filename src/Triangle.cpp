#include "Triangle.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// ---- Shaders ---------------------------------------------------------------

static const char* VERT_SRC = R"(
    #version 450 core
    layout(location = 0) in vec3 aPos;
    uniform mat4 MVP;
    void main() {
        gl_Position = MVP * vec4(aPos, 1.0);
    }
)";

static const char* FRAG_SRC = R"(
    #version 450 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(0.2, 0.6, 1.0, 1.0);
    }
)";

// ---- Implémentation --------------------------------------------------------

unsigned int Triangle::compileShader(unsigned int type, const char* source)
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

Triangle::Triangle()
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

Triangle::~Triangle()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
}

void Triangle::draw(const glm::mat4& MVP)
{
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "MVP"),
        1, GL_FALSE, glm::value_ptr(MVP));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}