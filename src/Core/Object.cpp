#include "Object.h"
#include <glm/gtc/matrix_inverse.hpp>

// ---- Wireframe shader -------------------------------------------------------

static const char* WIREFRAME_VERT = R"(
    #version 450 core
    layout(location = 0) in vec3 aPos;
    uniform mat4 uMVP;
    void main() {
        gl_Position = uMVP * vec4(aPos, 1.0);
    }
)";

static const char* WIREFRAME_FRAG = R"(
    #version 450 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(1.0, 1.0, 1.0, 0.35);
    }
)";

static Material* wireframeMaterial()
{
    static std::unique_ptr<Material> mat;
    if (!mat)
        mat = std::make_unique<Material>(WIREFRAME_VERT, WIREFRAME_FRAG);
    return mat.get();
}

// ---- Outline shader (partagé entre tous les Object) ------------------------

static const char* OUTLINE_VERT = R"(
    #version 450 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec3 aNormal;
    uniform mat4  uMVP;
    uniform float uThickness;
    void main() {
        // Expansion en espace modèle le long de la normale
        gl_Position = uMVP * vec4(aPos + normalize(aNormal) * uThickness, 1.0);
    }
)";

static const char* OUTLINE_FRAG = R"(
    #version 450 core
    uniform vec3 uOutlineColor;
    out vec4 FragColor;
    void main() {
        FragColor = vec4(uOutlineColor, 1.0);
    }
)";

static Material* outlineMaterial()
{
    static std::unique_ptr<Material> mat;
    if (!mat)
        mat = std::make_unique<Material>(OUTLINE_VERT, OUTLINE_FRAG);
    return mat.get();
}

// ---- Object ----------------------------------------------------------------

Object::Object(std::unique_ptr<Shape> s, std::unique_ptr<Material> m)
    : shape(std::move(s)), material(std::move(m))
{}

void Object::draw(const DrawContext& ctx)
{
    const glm::mat4 model        = transform.getMatrix();
    const glm::mat4 MVP          = ctx.proj * ctx.view * model;
    const glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(model));

    if (selected)
    {
        // Passe 1 : dessin normal + écriture dans le stencil buffer (valeur 1)
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilMask(0xFF);
    }

    // Dessin normal (toujours exécuté)
    material->use();
    material->set("uMVP",           MVP);
    material->set("uModel",         model);
    material->set("uNormalMatrix",  normalMatrix);
    material->set("uViewPos",       ctx.viewPos);
    material->set("uLightPos",      ctx.light.position);
    material->set("uLightColor",    ctx.light.color);
    material->set("uLightIntensity", ctx.light.intensity);
    shape->draw();

    if (selected)
    {
        // Passe 2 : outline uniquement là où le stencil vaut 0 (hors de l'objet)
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);

        Material* outline = outlineMaterial();
        outline->use();
        outline->set("uMVP",          MVP);
        outline->set("uThickness",    0.04f);
        outline->set("uOutlineColor", glm::vec3(1.0f, 0.55f, 0.0f)); // orange

        shape->draw();

        // Restauration
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glDisable(GL_STENCIL_TEST);
        glEnable(GL_DEPTH_TEST);
    }

    if (wireframe)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_DEPTH_TEST);

        Material* wf = wireframeMaterial();
        wf->use();
        wf->set("uMVP", MVP);
        shape->draw();

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }
}
