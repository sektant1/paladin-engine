#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "Types.h"
#include "graphics/VertexLayout.h"
#include "render/Mesh.h"

namespace ENG
{

struct MeshData
{
    std::vector<f32> vertices;
    std::vector<u32> indices;
    VertexLayout     layout;

    std::unique_ptr<Mesh> buildMesh();
};

class Builder
{
public:
    static MeshData CreateRectangle(f32 width, f32 height);
    static MeshData CreateTriangle(f32 size);
    static MeshData CreateFullscreenQuad();
};

}  // namespace ENG
