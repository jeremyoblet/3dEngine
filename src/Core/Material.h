#pragma once
#include <string>
#include <unordered_map>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Material {
public:
    Material(const char* vertSrc, const char* fragSrc);
    ~Material();

    void use() const;

    void set(const std::string& name, const glm::mat4&) const;
    void set(const std::string& name, const glm::mat3&) const;
    void set(const std::string& name, const glm::vec3&) const;
    void set(const std::string& name, float)            const;

private:
    unsigned int program;
    mutable std::unordered_map<std::string, int> locationCache;

    int location(const std::string& name) const;
    static unsigned int compile(unsigned int type, const char* src);
};
