#include <memory>

#include "render/Builder.h"

#include "render/Mesh.h"

namespace COA
{

std::shared_ptr<Mesh> MeshData::buildMesh()
{
    return std::make_shared<Mesh>(layout, vertices, indices);
}

MeshData Builder::CreateCube(f32 width, f32 height)
{
    MeshData data;

    // clang-format off
    data.vertices = {
        // Front face
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

        // Top face 
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

        // Right face
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

        // Left face
        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

        // Bottom face
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

        // Back face
        -0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f
    };
    // clang-format on

    // clang-format off
    data.indices = {
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

    // Postion
    data.layout.elements.push_back({0, 3, GL_FLOAT, 0});

    // Color
    data.layout.elements.push_back({1, 3, GL_FLOAT, sizeof(f32) * 3});

    // UV
    data.layout.elements.push_back({2, 2, GL_FLOAT, sizeof(f32) * 6});

    data.layout.stride = sizeof(f32) * 8;

    return data;
}

MeshData Builder::CreateRectangle(f32 width, f32 height)
{
    MeshData data;

    f32 halfW = width / 2.0F;
    f32 halfH = height / 2.0F;

    // clang-format off
    data.vertices = {
        halfW,  halfH, 0.0F, 1.0F, 0.0F, 0.0F, // top-right
       -halfW,  halfH, 0.0F, 0.0F, 1.0F, 0.0F, // top-left
       -halfW, -halfH, 0.0F, 0.0F, 0.0F, 1.0F, // bottom-left
        halfW, -halfH, 0.0f, 1.0f, 1.0f, 0.0f  // bottom-right
    };
    // clang-format on

    data.indices = {0, 1, 2, 0, 2, 3};

    // Postion
    data.layout.elements.push_back({0, 3, GL_FLOAT, 0});
    // Color
    data.layout.elements.push_back({1, 3, GL_FLOAT, sizeof(f32) * 3});
    data.layout.stride = sizeof(f32) * 6;

    return data;
}

MeshData Builder::CreateTriangle(f32 size)
{
    MeshData data;

    // clang-format off
    data.vertices = {
       -size, -size, 0.0F, 1.0F, 0.0F, 0.0F, //bottom-left, red  
        size, -size, 0.0F, 0.0F, 1.0F, 0.0F, //bottom-right, green  
        0.0F,  size, 0.0F, 0.0F, 0.0F, 1.0F, //top, blue  
    };
    // clang-format on

    data.indices = {
        0,
        1,
        2,
    };

    // Postion
    data.layout.elements.push_back({0, 3, GL_FLOAT, 0});
    // Color
    data.layout.elements.push_back({1, 3, GL_FLOAT, sizeof(f32) * 3});
    data.layout.stride = sizeof(f32) * 6;

    return data;
}

MeshData Builder::CreateFullscreenQuad()
{
    MeshData data;
    // clang-format off
    data.vertices = {
        -1.0F, -1.0F, 0.0F, 1.0F, 1.0F, 1.0F, // bottom-left
         1.0F, -1.0F, 0.0F, 1.0F, 1.0F, 1.0F, // bottom-right
         1.0F,  1.0F, 0.0F, 1.0F, 1.0F, 1.0F, // top-right
        -1.0F,  1.0F, 0.0F, 1.0F, 1.0F, 1.0F  // top-left
    };
    // clang-format on
    data.indices = {0, 1, 2, 0, 2, 3};
    // Position
    data.layout.elements.push_back({0, 3, GL_FLOAT, 0});
    // Color (unused by shader effect, but keeps layout consistent)
    data.layout.elements.push_back({1, 3, GL_FLOAT, sizeof(f32) * 3});
    data.layout.stride = sizeof(f32) * 6;
    return data;
}
}  // namespace COA
