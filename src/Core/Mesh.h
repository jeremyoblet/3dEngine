#pragma once
#include <vector>
#include <glad/glad.h>
#include "Shape.h"

class Mesh : public Shape {
public:
    struct Attrib {
        unsigned int location;
        int          size;    // nombre de composantes (2, 3, 4)
        int          offset;  // offset en nombre de floats depuis le début du vertex
    };

    Mesh(const std::vector<float>& vertices, int strideFloats,
         const std::vector<Attrib>& attribs);
    ~Mesh() override;

    void draw() const override;

private:
    unsigned int VAO, VBO;
    int          vertexCount;
};
