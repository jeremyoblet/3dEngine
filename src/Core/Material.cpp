#include "Material.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

unsigned int Material::compile(unsigned int type, const char* src)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader error: " << log << std::endl;
    }
    return shader;
}

Material::Material(const char* vertSrc, const char* fragSrc)
{
    unsigned int vert = compile(GL_VERTEX_SHADER,   vertSrc);
    unsigned int frag = compile(GL_FRAGMENT_SHADER, fragSrc);

    program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);
    glDeleteShader(vert);
    glDeleteShader(frag);
}

Material::~Material()
{
    glDeleteProgram(program);
}

int Material::location(const std::string& name) const
{
    auto it = locationCache.find(name);
    if (it != locationCache.end()) return it->second;
    int loc = glGetUniformLocation(program, name.c_str());
    locationCache[name] = loc;
    return loc;
}

void Material::use() const
{
    glUseProgram(program);
}

void Material::set(const std::string& name, const glm::mat4& v) const
{
    glUniformMatrix4fv(location(name), 1, GL_FALSE, glm::value_ptr(v));
}

void Material::set(const std::string& name, const glm::mat3& v) const
{
    glUniformMatrix3fv(location(name), 1, GL_FALSE, glm::value_ptr(v));
}

void Material::set(const std::string& name, const glm::vec3& v) const
{
    glUniform3fv(location(name), 1, glm::value_ptr(v));
}

void Material::set(const std::string& name, float v) const
{
    glUniform1f(location(name), v);
}

void Material::set(const std::string& name, int v) const
{
    glUniform1i(location(name), v);
}
