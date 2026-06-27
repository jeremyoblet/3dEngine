#include "Cube.h"
#include "Core/Mesh.h"
#include "Core/Material.h"

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
        vec3 ambient = 0.12 * vColor;

        vec3 norm     = normalize(vNormal);
        vec3 lightDir = normalize(uLightPos - vWorldPos);
        float diff    = max(dot(norm, lightDir), 0.0);

        float d           = length(uLightPos - vWorldPos);
        float attenuation = uLightIntensity / (1.0 + 0.35 * d + 0.44 * d * d);

        vec3 diffuse = diff * uLightColor * vColor * attenuation;

        vec3 viewDir  = normalize(uViewPos - vWorldPos);
        vec3 halfDir  = normalize(lightDir + viewDir);
        float spec    = pow(max(dot(norm, halfDir), 0.0), 64.0);
        vec3 specular = 0.4 * spec * uLightColor * attenuation;

        FragColor = vec4(ambient + diffuse + specular, 1.0);
    }
)";

// ---- Géométrie : position (3) + normale (3) + couleur (3) = 9 floats ------

static const std::vector<float> VERTICES = {
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

// ---- Constructeur ----------------------------------------------------------

Cube::Cube()
    : Object(
        std::make_unique<Mesh>(VERTICES, 9,
            std::vector<Mesh::Attrib>{
                { 0, 3, 0 },   // position
                { 1, 3, 3 },   // normale
                { 2, 3, 6 },   // couleur
            }),
        std::make_unique<Material>(VERT_SRC, FRAG_SRC)
    )
{}
