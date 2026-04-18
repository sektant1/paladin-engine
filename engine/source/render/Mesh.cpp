#include "render/Mesh.h"

#include "Engine.h"
#include "Log.h"
#include "graphics/GraphicsAPI.h"

namespace ENG
{

Mesh::Mesh(const VertexLayout &layout, const std::vector<float> &vertices, const std::vector<uint32_t> &indices)
{
    m_vertexLayout = layout;

    auto &graphicsAPI = Engine::GetInstance().GetGraphicsAPI();

    m_VBO = graphicsAPI.CreateVertexBuffer(vertices);
    m_EBO = graphicsAPI.CreateIndexBuffer(indices);

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    for (auto &element : m_vertexLayout.elements) {
        glVertexAttribPointer(element.index,
                              element.size,
                              element.type,
                              GL_FALSE,
                              m_vertexLayout.stride,
                              (void *)(uintptr_t)element.offset);
        glEnableVertexAttribArray(element.index);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    m_vertexCount = (vertices.size() * sizeof(float)) / m_vertexLayout.stride;
    m_indexCount  = indices.size();

    LOG_INFO("Mesh created (VAO=%u VBO=%u EBO=%u verts=%zu indices=%zu)",
             m_VAO,
             m_VBO,
             m_EBO,
             m_vertexCount,
             m_indexCount);
}

Mesh::Mesh(const VertexLayout &layout, const std::vector<float> &vertices)
{
    m_vertexLayout = layout;

    auto &graphicsAPI = Engine::GetInstance().GetGraphicsAPI();

    m_VBO = graphicsAPI.CreateVertexBuffer(vertices);

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    for (auto &element : m_vertexLayout.elements) {
        glVertexAttribPointer(element.index,
                              element.size,
                              element.type,
                              GL_FALSE,
                              m_vertexLayout.stride,
                              (void *)(uintptr_t)element.offset);
        glEnableVertexAttribArray(element.index);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_vertexCount = (vertices.size() * sizeof(float)) / m_vertexLayout.stride;

    LOG_INFO("Mesh created (VAO=%u VBO=%u verts=%zu, non-indexed)", m_VAO, m_VBO, m_vertexCount);
}

void Mesh::Bind()
{
    glBindVertexArray(m_VAO);
}

void Mesh::Draw()
{
    if (m_indexCount > 0) {
        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
    }
}

}  // namespace ENG
