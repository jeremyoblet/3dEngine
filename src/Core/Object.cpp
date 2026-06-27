#include "Object.h"
#include <glm/gtc/matrix_inverse.hpp>

Object::Object(std::unique_ptr<Shape> s, std::unique_ptr<Material> m)
    : shape(std::move(s)), material(std::move(m))
{}

void Object::draw(const DrawContext& ctx)
{
    const glm::mat4 model        = transform.getMatrix();
    const glm::mat4 MVP          = ctx.proj * ctx.view * model;
    const glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(model));

    material->use();
    material->set("uMVP",           MVP);
    material->set("uModel",         model);
    material->set("uNormalMatrix",  normalMatrix);
    material->set("uViewPos",       ctx.viewPos);
    material->set("uLightPos",      ctx.light.position);
    material->set("uLightColor",    ctx.light.color);
    material->set("uLightIntensity", ctx.light.intensity);

    shape->draw();
}
