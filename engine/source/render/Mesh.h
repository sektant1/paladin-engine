#pragma once
#include <memory>
#include <vector>

#include "GL/glew.h"
#include "Types.h"
#include "graphics/VertexLayout.h"

namespace ENG
{

class Mesh
{
public:
    Mesh(const VertexLayout &layout, const std::vector<float> &vertices, const std::vector<uint32_t> &indices);
    Mesh(const VertexLayout &layout, const std::vector<float> &vertices);
    Mesh(const Mesh &)            = delete;
    Mesh &operator=(const Mesh &) = delete;

    void Bind();
    void Draw();

    static std::shared_ptr<Mesh> CreateCube();
    // static std::shared_ptr<Mesh> Load(const str &path);

private:
    VertexLayout m_vertexLayout;

    GLuint m_VBO         = 0;
    GLuint m_EBO         = 0;
    GLuint m_VAO         = 0;
    size_t m_vertexCount = 0;
    size_t m_indexCount  = 0;
};
}  // namespace ENG
