#include "Cube.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <iostream>

// ---- Shaders ---------------------------------------------------------------

static const char* VERT_SRC = R"(
    #version 450 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec3 aNormal;
    layout(location = 2) in vec3 aColor;

    uniform mat4 uMVP;
    uniform mat4 uModel;
    uniform mat3 uNormalMatrix;

    out vec3 vWorldPos;
    out vec3 vNormal;
    out vec3 vColor;

    void main() {
        vWorldPos = vec3(uModel * vec4(aPos, 1.0));
        vNormal   = normalize(uNormalMatrix * aNormal);
        vColor    = aColor;
        gl_Position = uMVP * vec4(aPos, 1.0);
    }
)";

static const char* FRAG_SRC = R"(
    #version 450 core
    in vec3 vWorldPos;
    in vec3 vNormal;
    in vec3 vColor;

    uniform vec3  uLightPos;
    uniform vec3  uLightColor;
    uniform float uLightIntensity;
    uniform vec3  uViewPos;

    out vec4 FragColor;

    void main() {
        // Ambient
        vec3 ambient = 0.12 * vColor;

        // Diffuse
        vec3 norm     = normalize(vNormal);
        vec3 lightDir = normalize(uLightPos - vWorldPos);
        float diff    = max(dot(norm, lightDir), 0.0);

        // Attenuation (quadratique)
        float d           = length(uLightPos - vWorldPos);
        float attenuation = uLightIntensity / (1.0 + 0.35 * d + 0.44 * d * d);

        vec3 diffuse = diff * uLightColor * vColor * attenuation;

        // Specular (Blinn-Phong)
        vec3 viewDir    = normalize(uViewPos - vWorldPos);
        vec3 halfDir    = normalize(lightDir + viewDir);
        float spec      = pow(max(dot(norm, halfDir), 0.0), 64.0);
        vec3 specular   = 0.4 * spec * uLightColor * attenuation;

        FragColor = vec4(ambient + diffuse + specular, 1.0);
    }
)";

// ---- Géométrie : position (3) + normale (3) + couleur (3) = 9 floats ------

static constexpr float VERTICES[] = {
    // Face avant  (z = +0.5)  normale (0,0,1)  rouge
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.2f, 0.2f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.2f, 0.2f,
     0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.2f, 0.2f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.2f, 0.2f,
     0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.2f, 0.2f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.2f, 0.2f,

    // Face arrière (z = -0.5)  normale (0,0,-1)  vert
     0.5f, -0.5f, -0.5f,  0.0f, 0.0f,-1.0f,  0.2f, 1.0f, 0.2f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,-1.0f,  0.2f, 1.0f, 0.2f,
    -0.5f,  0.5f, -0.5f,  0.0f, 0.0f,-1.0f,  0.2f, 1.0f, 0.2f,
     0.5f, -0.5f, -0.5f,  0.0f, 0.0f,-1.0f,  0.2f, 1.0f, 0.2f,
    -0.5f,  0.5f, -0.5f,  0.0f, 0.0f,-1.0f,  0.2f, 1.0f, 0.2f,
     0.5f,  0.5f, -0.5f,  0.0f, 0.0f,-1.0f,  0.2f, 1.0f, 0.2f,

    // Face gauche (x = -0.5)  normale (-1,0,0)  bleu
    -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,  0.2f, 0.2f, 1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f,  0.2f, 0.2f, 1.0f,
    -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f,  0.2f, 0.2f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,  0.2f, 0.2f, 1.0f,
    -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f,  0.2f, 0.2f, 1.0f,
    -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f,  0.2f, 0.2f, 1.0f,

    // Face droite (x = +0.5)  normale (1,0,0)  jaune
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f, 0.2f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f, 0.2f,
     0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f, 0.2f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f, 0.2f,
     0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f, 0.2f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f, 0.2f,

    // Face haut   (y = +0.5)  normale (0,1,0)  cyan
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.2f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.2f, 1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.2f, 1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.2f, 1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.2f, 1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.2f, 1.0f, 1.0f,

    // Face bas    (y = -0.5)  normale (0,-1,0)  magenta
    -0.5f, -0.5f, -0.5f,  0.0f,-1.0f, 0.0f,  1.0f, 0.2f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,-1.0f, 0.0f,  1.0f, 0.2f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,-1.0f, 0.0f,  1.0f, 0.2f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,-1.0f, 0.0f,  1.0f, 0.2f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,-1.0f, 0.0f,  1.0f, 0.2f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,-1.0f, 0.0f,  1.0f, 0.2f, 1.0f,
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

    constexpr int STRIDE = 9 * sizeof(float);

    // location 0 : position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)0);
    glEnableVertexAttribArray(0);

    // location 1 : normale
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // location 2 : couleur
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

Cube::~Cube()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
}

void Cube::draw(const glm::mat4& MVP, const glm::mat4& model,
                const PointLight& light, const glm::vec3& viewPos)
{
    glUseProgram(shaderProgram);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uMVP"),
        1, GL_FALSE, glm::value_ptr(MVP));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uModel"),
        1, GL_FALSE, glm::value_ptr(model));

    const glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(model));
    glUniformMatrix3fv(glGetUniformLocation(shaderProgram, "uNormalMatrix"),
        1, GL_FALSE, glm::value_ptr(normalMatrix));

    glUniform3fv(glGetUniformLocation(shaderProgram, "uLightPos"),
        1, glm::value_ptr(light.position));
    glUniform3fv(glGetUniformLocation(shaderProgram, "uLightColor"),
        1, glm::value_ptr(light.color));
    glUniform1f(glGetUniformLocation(shaderProgram, "uLightIntensity"),
        light.intensity);

    glUniform3fv(glGetUniformLocation(shaderProgram, "uViewPos"),
        1, glm::value_ptr(viewPos));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
