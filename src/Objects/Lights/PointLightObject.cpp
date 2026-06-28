#include "PointLightObject.h"
#include "Core/Mesh.h"
#include "Core/Material.h"
#include <glm/glm.hpp>
#include <vector>

static const char* VERT_SRC = R"(
    #version 450 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec3 aNormal;
    layout(location = 2) in vec3 aColor;
    uniform mat4 uMVP;
    out vec3 vColor;
    void main() {
        vColor      = aColor;
        gl_Position = uMVP * vec4(aPos, 1.0);
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

static std::vector<float> generateVertices()
{
    constexpr float R = 0.15f;

    const glm::vec3 T  = {  0,  R,  0 };
    const glm::vec3 B  = {  0, -R,  0 };
    const glm::vec3 Ri = {  R,  0,  0 };
    const glm::vec3 Le = { -R,  0,  0 };
    const glm::vec3 Fr = {  0,  0,  R };
    const glm::vec3 Bk = {  0,  0, -R };

    std::vector<float> v;
    v.reserve(8 * 3 * 9);

    auto push = [&](const glm::vec3& p, const glm::vec3& n) {
        v.push_back(p.x); v.push_back(p.y); v.push_back(p.z);
        v.push_back(n.x); v.push_back(n.y); v.push_back(n.z);
        v.push_back(1.0f); v.push_back(0.9f); v.push_back(0.3f); // jaune
    };

    auto addFace = [&](const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
        glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));
        push(a, n); push(b, n); push(c, n);
    };

    // demi-sphère haute
    addFace(T, Fr, Ri);
    addFace(T, Ri, Bk);
    addFace(T, Bk, Le);
    addFace(T, Le, Fr);
    // demi-sphère basse
    addFace(B, Ri, Fr);
    addFace(B, Bk, Ri);
    addFace(B, Le, Bk);
    addFace(B, Fr, Le);

    return v;
}

PointLightObject::PointLightObject()
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
