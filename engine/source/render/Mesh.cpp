#include <memory>
#include <vector>

#include "render/Mesh.h"

#include "Constants.h"
#include "Engine.h"
#include "Log.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/VertexLayout.h"

namespace COA
{

Mesh::Mesh(const VertexLayout &layout, const std::vector<float> &vertices, const std::vector<uint32_t> &indices)
    : m_vertexLayout(layout)
{
    auto &graphicsAPI = Engine::GetInstance().GetGraphicsAPI();

    m_VBO = graphicsAPI.CreateVertexBuffer(vertices);
    m_EBO = graphicsAPI.CreateIndexBuffer(indices);

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    for (auto &element : m_vertexLayout.elements)
    {
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

    LOG_INFO(
        "Mesh created (VAO=%u VBO=%u EBO=%u verts=%zu indices=%zu)", m_VAO, m_VBO, m_EBO, m_vertexCount, m_indexCount);
}

std::shared_ptr<Mesh> Mesh::CreateCube()
{
    // clang-format off
    std::vector<f32> vertices = {
        // Front face (+Z)
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f,0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,

        // Top face (+Y)
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,

        // Right face (+X)
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,1.0f, 0.0f, 0.0f,

        // Left face (-X)
        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,-1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,

        // Bottom face (-Y)
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,

        // Back face (-Z)
        -0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,  0.0f, 0.0f, -1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,0.0f, 0.0f, -1.0f,
    };

    // clang-format off
    std::vector<uint32_t> indices = {
        // front face
        0, 1, 2,
        0, 2, 3,
        // top face
        4, 5, 6,
        4, 6, 7,
        // right face
        8, 9, 10,
        8, 10, 11,
        // left face
        12, 13, 14,
        12, 14, 15,
        // bottom face
        16, 17, 18,
        16, 18, 19,
        // back face
        20, 21, 22,
        20, 22, 23
    };

    // clang-format on

    VertexLayout layout;

    // Postion
    layout.elements.push_back({VertexElement::PositionIndex, 3, GL_FLOAT, 0});

    // Color
    layout.elements.push_back({VertexElement::ColorIndex, 3, GL_FLOAT, sizeof(f32) * 3});

    // UV
    layout.elements.push_back({VertexElement::UVIndex, 2, GL_FLOAT, sizeof(f32) * 6});

    // Normal
    layout.elements.push_back({VertexElement::NormalIndex, 3, GL_FLOAT, sizeof(f32) * 8});

    layout.stride = sizeof(f32) * kStandardVertexFloats;

    auto result = std::make_shared<Mesh>(layout, vertices, indices);

    return result;
}

Mesh::Mesh(const VertexLayout &layout, const std::vector<float> &vertices)
{
    m_vertexLayout = layout;

    auto &graphicsAPI = Engine::GetInstance().GetGraphicsAPI();

    m_VBO = graphicsAPI.CreateVertexBuffer(vertices);

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    for (auto &element : m_vertexLayout.elements)
    {
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
    if (m_indexCount > 0)
    {
        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
    } else
    {
        glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
    }
}

std::shared_ptr<Mesh> Mesh::CreateBox(const glm::vec3 &extents)
{
    const glm::vec3 half = extents * 0.5f;
    // UVs tile one repeat per world unit on each face's tangent axes.
    const float        ux = extents.x, uy = extents.y, uz = extents.z;
    std::vector<float> vertices = {

        // clang-format off
            // Front face (tangent axes: x, y)
            half.x, half.y, half.z, 1.0f, 0.0f, 0.0f,   ux,   uy, 0.0f, 0.0f, 1.0f,
            -half.x, half.y, half.z, 0.0f, 1.0f, 0.0f, 0.0f,   uy, 0.0f, 0.0f, 1.0f,
            -half.x, -half.y, half.z, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            half.x, -half.y, half.z, 1.0f, 1.0f, 0.0f,   ux, 0.0f, 0.0f, 0.0f, 1.0f,

            // Top face (tangent axes: x, z)
            half.x, half.y, -half.z, 1.0f, 0.0f, 0.0f,   ux,   uz, 0.0f, 1.0f, 0.0f,
            -half.x, half.y, -half.z, 0.0f, 1.0f, 0.0f, 0.0f,   uz, 0.0f, 1.0f, 0.0f,
            -half.x, half.y, half.z, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            half.x, half.y, half.z, 1.0f, 1.0f, 0.0f,   ux, 0.0f, 0.0f, 1.0f, 0.0f,

            // Right face (tangent axes: z, y)
            half.x, half.y, -half.z, 1.0f, 0.0f, 0.0f,   uz,   uy, 1.0f, 0.0f, 0.0f,
            half.x, half.y, half.z, 0.0f, 1.0f, 0.0f, 0.0f,   uy, 1.0f, 0.0f, 0.0f,
            half.x, -half.y, half.z, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            half.x, -half.y, -half.z, 1.0f, 1.0f, 0.0f,   uz, 0.0f, 1.0f, 0.0f, 0.0f,

            // Left face (tangent axes: z, y)
            -half.x, half.y, half.z, 1.0f, 0.0f, 0.0f,   uz,   uy, -1.0f, 0.0f, 0.0f,
            -half.x, half.y, -half.z, 0.0f, 1.0f, 0.0f, 0.0f,   uy, -1.0f, 0.0f, 0.0f,
            -half.x, -half.y, -half.z, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
            -half.x, -half.y, half.z, 1.0f, 1.0f, 0.0f,   uz, 0.0f, -1.0f, 0.0f, 0.0f,

            // Bottom face (tangent axes: x, z)
            half.x, -half.y, half.z, 1.0f, 0.0f, 0.0f,   ux,   uz, 0.0f, -1.0f, 0.0f,
            -half.x, -half.y, half.z, 0.0f, 1.0f, 0.0f, 0.0f,   uz, 0.0f, -1.0f, 0.0f,
            -half.x, -half.y, -half.z, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
            half.x, -half.y, -half.z, 1.0f, 1.0f, 0.0f,   ux, 0.0f, 0.0f, -1.0f, 0.0f,

            // Back face (tangent axes: x, y)
            -half.x, half.y, -half.z, 1.0f, 0.0f, 0.0f,   ux,   uy, 0.0f, 0.0f, -1.0f,
            half.x, half.y, -half.z, 0.0f, 1.0f, 0.0f, 0.0f,   uy, 0.0f, 0.0f, -1.0f,
            half.x, -half.y, -half.z, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
            -half.x, -half.y, -half.z, 1.0f, 1.0f, 0.0f,   ux, 0.0f, 0.0f, 0.0f, -1.0f
        };

        std::vector<unsigned int> indices =
        {
            // front face
            0, 1, 2,
            0, 2, 3,
            // top face
            4, 5, 6,
            4, 6, 7,
            // right face
            8, 9, 10,
            8, 10, 11,
            // left face
            12, 13, 14,
            12, 14, 15,
            // bottom face
            16, 17, 18,
            16, 18, 19,
            // back face
            20, 21, 22,
            20, 22, 23
        };

        COA::VertexLayout vertexLayout;

        // Postion
        vertexLayout.elements.push_back({
            VertexElement::PositionIndex,
            3,
            GL_FLOAT,
            0
            });
        // Color
        vertexLayout.elements.push_back({
            VertexElement::ColorIndex,
            3,
            GL_FLOAT,
            sizeof(float) * 3
            });
        // UV
        vertexLayout.elements.push_back({
            VertexElement::UVIndex,
            2,
            GL_FLOAT,
            sizeof(float) * 6
            });
        // Normal
        vertexLayout.elements.push_back({
            VertexElement::NormalIndex,
            3,
            GL_FLOAT,
            sizeof(float) * 8
            });

    // clang-format on
    vertexLayout.stride = sizeof(float) * kStandardVertexFloats;

    auto result = std::make_shared<COA::Mesh>(vertexLayout, vertices, indices);

    return result;
}

void Mesh::Unbind()
{
    glBindVertexArray(0);
}

std::shared_ptr<Mesh> Mesh::CreateSphere(f32 radius, int sectors, int stacks)
{
    std::vector<float> vertices((stacks + 1) * (sectors + 1) * 8);
    for (int i = 0; i <= stacks; ++i)
    {
        float stackAngle = kPI / 2.0f - static_cast<float>(i) * (kPI / static_cast<float>(stacks));  // From -π/2 to π/2
        float xy         = radius * cosf(stackAngle);  // x-y plane radius at this stack
        float z          = radius * sinf(stackAngle);  // z coordinate

        for (int j = 0; j <= sectors; ++j)
        {
            float sectorAngle = static_cast<float>(j) * (2.0f * kPI / static_cast<float>(sectors));  // From 0 to 2π

            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            size_t vertexStart = (i * (sectors + 1) + j) * 8;
            // Position
            vertices[vertexStart]     = x;
            vertices[vertexStart + 1] = y;
            vertices[vertexStart + 2] = z;

            // Normal (normalized position vector)
            float length              = sqrtf(x * x + y * y + z * z);
            vertices[vertexStart + 3] = x / length;
            vertices[vertexStart + 4] = y / length;
            vertices[vertexStart + 5] = z / length;

            // UV coordinates
            vertices[vertexStart + 6] = static_cast<float>(j) / static_cast<float>(sectors);
            vertices[vertexStart + 7] = static_cast<float>(i) / static_cast<float>(stacks);
        }
    }

    // Generate indices
    std::vector<unsigned int> indices;
    for (int i = 0; i < stacks; ++i)
    {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;

        for (int j = 0; j < sectors; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stacks - 1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    COA::VertexLayout vertexLayout;

    // Postion
    vertexLayout.elements.push_back({VertexElement::PositionIndex, 3, GL_FLOAT, 0});
    // Normal
    vertexLayout.elements.push_back({VertexElement::NormalIndex, 3, GL_FLOAT, sizeof(float) * 3});
    // UV
    vertexLayout.elements.push_back({VertexElement::UVIndex, 2, GL_FLOAT, sizeof(float) * 6});
    vertexLayout.stride = sizeof(float) * 8;

    auto result = std::make_shared<COA::Mesh>(vertexLayout, vertices, indices);

    return result;
}

}  // namespace COA
