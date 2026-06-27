#include "Mesh.h"

Mesh::Mesh(const std::vector<float>& vertices, int strideFloats,
           const std::vector<Attrib>& attribs)
    : vertexCount(static_cast<int>(vertices.size()) / strideFloats)
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
                 vertices.data(), GL_STATIC_DRAW);

    const int stride = strideFloats * static_cast<int>(sizeof(float));
    for (const auto& a : attribs) {
        glVertexAttribPointer(a.location, a.size, GL_FLOAT, GL_FALSE,
                              stride, reinterpret_cast<void*>(a.offset * sizeof(float)));
        glEnableVertexAttribArray(a.location);
    }

    glBindVertexArray(0);
}

Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Mesh::draw() const
{
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}
