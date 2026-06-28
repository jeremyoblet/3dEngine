#include "Sphere.h"
#include "Core/Mesh.h"
#include "Core/Material.h"
#include <cmath>
#include <vector>

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
        vWorldPos   = vec3(uModel * vec4(aPos, 1.0));
        vNormal     = normalize(uNormalMatrix * aNormal);
        vColor      = aColor;
        gl_Position = uMVP * vec4(aPos, 1.0);
    }
)";

static const char* FRAG_SRC = R"(
    #version 450 core
    in vec3 vWorldPos;
    in vec3 vNormal;
    in vec3 vColor;

    #define MAX_LIGHTS 4
    uniform int   uNumLights;
    uniform vec3  uLightPos[MAX_LIGHTS];
    uniform vec3  uLightColor[MAX_LIGHTS];
    uniform float uLightIntensity[MAX_LIGHTS];
    uniform vec3  uViewPos;

    out vec4 FragColor;

    void main() {
        vec3 norm    = normalize(vNormal);
        vec3 viewDir = normalize(uViewPos - vWorldPos);
        vec3 result  = 0.12 * vColor; // ambient
        for (int i = 0; i < uNumLights; i++) {
            vec3  lightDir    = normalize(uLightPos[i] - vWorldPos);
            float diff        = max(dot(norm, lightDir), 0.0);
            float d           = length(uLightPos[i] - vWorldPos);
            float attenuation = uLightIntensity[i] / (1.0 + 0.35 * d + 0.44 * d * d);
            vec3  diffuse     = diff * uLightColor[i] * vColor * attenuation;
            vec3  halfDir     = normalize(lightDir + viewDir);
            float spec        = pow(max(dot(norm, halfDir), 0.0), 64.0);
            vec3  specular    = 0.4 * spec * uLightColor[i] * attenuation;
            result += diffuse + specular;
        }
        FragColor = vec4(result, 1.0);
    }
)";

static std::vector<float> generateVertices()
{
    constexpr int   RINGS   = 16;
    constexpr int   SECTORS = 32;
    constexpr float PI      = 3.14159265358979f;
    constexpr float R       = 0.5f; // rayon → bounding box de 1 unité

    std::vector<float> v;
    v.reserve(6 * RINGS * SECTORS * 9);

    auto push = [&](float phi, float theta) {
        float nx = std::sin(phi) * std::cos(theta);
        float ny = std::cos(phi);
        float nz = std::sin(phi) * std::sin(theta);
        v.push_back(nx * R); v.push_back(ny * R); v.push_back(nz * R); // position
        v.push_back(nx);     v.push_back(ny);     v.push_back(nz);     // normale
        v.push_back(0.2f);   v.push_back(0.8f);   v.push_back(0.7f);  // couleur teal
    };

    for (int r = 0; r < RINGS; ++r) {
        for (int s = 0; s < SECTORS; ++s) {
            float phi0   = PI * r       / RINGS;
            float phi1   = PI * (r + 1) / RINGS;
            float theta0 = 2.0f * PI * s       / SECTORS;
            float theta1 = 2.0f * PI * (s + 1) / SECTORS;

            push(phi0, theta0); push(phi1, theta0); push(phi1, theta1);
            push(phi0, theta0); push(phi1, theta1); push(phi0, theta1);
        }
    }

    return v;
}

Sphere::Sphere()
    : Object(
        std::make_unique<Mesh>(generateVertices(), 9,
            std::vector<Mesh::Attrib>{
                { 0, 3, 0 },
                { 1, 3, 3 },
                { 2, 3, 6 },
            }),
        std::make_unique<Material>(VERT_SRC, FRAG_SRC)
    )
{}
