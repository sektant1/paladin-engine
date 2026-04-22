#include <memory>
#include <vector>

#include "render/Mesh.h"

#include "Engine.h"
#include "Log.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/VertexLayout.h"

namespace ENG
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
    // clang-format on

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

    // clang-format off

    VertexLayout layout;

    // Postion
    layout.elements.push_back({VertexElement::PositionIndex, 3, GL_FLOAT, 0});

    // Color
    layout.elements.push_back({VertexElement::ColorIndex, 3, GL_FLOAT, sizeof(f32) * 3});

    // UV
    layout.elements.push_back({VertexElement::UVIndex, 2, GL_FLOAT, sizeof(f32) * 6});

    // Normal
    layout.elements.push_back({VertexElement::NormalIndex, 3, GL_FLOAT, sizeof(f32) * 8});

    layout.stride = sizeof(f32) * 11;

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
#if 0
std::shared_ptr<Mesh> Mesh::Load(const str &path)
{
    auto contents = Engine::GetInstance().GetFileSystem().LoadAssetFileText(path);

    if (contents.empty()) {
        return nullptr;
    }

    auto readFloats = [](const cgltf_accessor *acc, cgltf_size i, float *out, int n)
        {
            std::fill(out, out + n, 0.0f);
            return cgltf_accessor_read_float(acc, i, out, n) == 1;
        };

    auto readIndex = [](const cgltf_accessor *acc, cgltf_size i)
        {
            cgltf_uint out = 0;
            cgltf_bool ok  = cgltf_accessor_read_uint(acc, i, &out, 1);
            return ok ? static_cast<u32>(out) : 0;
        };

    cgltf_options options = {};
    cgltf_data   *data    = nullptr;
    cgltf_result  res     = cgltf_parse(&options, contents.data(), contents.size(), &data);

    if (res != cgltf_result_success) {
        return nullptr;
    }

    auto fullPath = Engine::GetInstance().GetFileSystem().GetAssetsFolder() / path;

    res = cgltf_load_buffers(&options, data, fullPath.remove_filename().string().c_str());

    if (res != cgltf_result_success) {
        cgltf_free(data);
        return nullptr;
    }

    std::shared_ptr<Mesh> result = nullptr;
    for (cgltf_size mi = 0; mi < data->meshes_count; ++mi) {
        auto mesh = data->meshes[mi];
        for (cgltf_size pi = 0; pi < mesh.primitives_count; ++pi) {
            auto &primitive = mesh.primitives[pi];
            if (primitive.type != cgltf_primitive_type_triangles) {
                continue;
            }

            VertexLayout    vertexLayout;
            cgltf_accessor *accessors[4] = {nullptr, nullptr, nullptr};

            for (cgltf_size ai = 0; ai < primitive.attributes_count; ++ai) {
                auto &attr = primitive.attributes[ai];
                auto  acc  = attr.data;
                if (!acc) {
                    continue;
                }

                VertexElement element;
                element.type = GL_FLOAT;

                switch (attr.type) {
                    case cgltf_attribute_type_position: {
                        accessors[VertexElement::PositionIndex] = acc;

                        element.index = VertexElement::PositionIndex;
                        element.size  = 3;
                    } break;
                    case cgltf_attribute_type_color: {
                        if (attr.index != 0) {
                            continue;
                        }
                        accessors[VertexElement::ColorIndex] = acc;

                        element.index = VertexElement::ColorIndex;
                        element.size  = 3;
                    } break;

                    case cgltf_attribute_type_texcoord: {
                        if (attr.index != 0) {
                            continue;
                        }
                        accessors[VertexElement::UVIndex] = acc;

                        element.index = VertexElement::UVIndex;
                        element.size  = 2;
                    } break;
                    case cgltf_attribute_type_normal: {
                        accessors[VertexElement::NormalIndex] = acc;

                        element.index = VertexElement::NormalIndex;
                        element.size  = 3;
                    } break;
                    default:
                        continue;
                }

                if (element.size > 0) {
                    element.offset = vertexLayout.stride;
                    vertexLayout.stride += element.size * sizeof(f32);
                    vertexLayout.elements.push_back(element);
                }
            }

            if (!accessors[VertexElement::PositionIndex]) {
                continue;
            }

            auto &vertexCount = accessors[VertexElement::PositionIndex]->count;

            std::vector<f32> vertices;
            vertices.resize(vertexLayout.stride / sizeof(f32) * vertexCount);

            for (cgltf_size vi = 0; vi < vertexCount; ++vi) {
                for (auto &elem : vertexLayout.elements) {
                    if (!accessors[elem.index]) {
                        continue;
                    }
                    auto index   = (vi * vertexLayout.stride + elem.offset) / sizeof(f32);
                    f32 *outData = &vertices[index];
                    readFloats(accessors[elem.index], vi, outData, elem.size);
                }
            }

            if (primitive.indices) {
                auto indexCount = primitive.indices->count;

                std::vector<u32> indices(indexCount);
                for (cgltf_size i = 0; i < indexCount; ++i) {
                    indices[i] = readIndex(primitive.indices, i);
                }

                result = std::make_shared<Mesh>(vertexLayout, vertices, indices);
            }

            else {
                result = std::make_shared<Mesh>(vertexLayout, vertices);
            }

            if (result) {
                break;
            }
        }

        if (result) {
            break;
        }
    }

    cgltf_free(data);

    return result;
}
#endif
}  // namespace ENG
