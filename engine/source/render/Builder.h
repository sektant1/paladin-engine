/**
 * @file Builder.h
 * @brief Static factory methods for creating common primitive mesh shapes.
 *
 * Builder produces MeshData — plain CPU-side geometry bundles (vertices,
 * indices, layout). Call buildMesh() on the result to upload to the GPU
 * and get a Mesh ready for rendering.
 *
 * ## Typical usage
 * @code
 *   MeshData data   = Builder::CreateRectangle(1.0f, 1.0f);
 *   auto     mesh   = data.buildMesh();   // uploads to GPU → shared_ptr<Mesh>
 *   // mesh is now ready to submit in a RenderCommand
 * @endcode
 *
 * All generated meshes include position, normal, and UV attributes laid out
 * according to VertexElement::PositionIndex / NormalIndex / UVIndex.
 *
 * @see MeshData, Mesh, VertexLayout
 */

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "Types.h"
#include "graphics/VertexLayout.h"
#include "render/Mesh.h"

namespace ENG
{

/**
 * @brief CPU-side mesh bundle produced by Builder factory methods.
 *
 * Holds raw vertex data, index data, and the VertexLayout that describes
 * how the interleaved bytes map to shader attributes. Call buildMesh() to
 * hand the data off to the GPU.
 */
struct MeshData
{
    std::vector<f32> vertices;  ///< Interleaved vertex data (pos, normal, UV …).
    std::vector<u32> indices;   ///< Triangle list indices (empty → non-indexed draw).
    VertexLayout     layout;    ///< Attribute layout that matches the vertex data format.

    /**
     * @brief Upload this geometry to the GPU and return a renderable Mesh.
     * @return Shared pointer to the uploaded Mesh.
     */
    std::shared_ptr<Mesh> buildMesh();
};

/**
 * @brief Factory that generates MeshData for common geometric primitives.
 *
 * All methods are static — Builder is never instantiated. Generated shapes
 * follow a consistent winding order (counter-clockwise front face) compatible
 * with the engine's default back-face culling state.
 */
class Builder
{
public:
    /**
     * @brief Create a flat quad with the given dimensions.
     * @param width  Extent along the X axis.
     * @param height Extent along the Y axis.
     */
    static MeshData CreateRectangle(f32 width, f32 height);

    /**
     * @brief Create a box (6 faces) with the given dimensions.
     * @param width  Extent along X.
     * @param height Extent along Y (also used for Z in the current implementation).
     */
    static MeshData CreateCube(f32 width, f32 height);

    /**
     * @brief Create an equilateral triangle centred at the origin.
     * @param size Side length.
     */
    static MeshData CreateTriangle(f32 size);

    /**
     * @brief Create a clip-space quad covering the entire screen.
     *
     * Useful for post-processing or Shadertoy-style full-screen fragment shaders.
     * The quad spans NDC [-1, 1] in both X and Y.
     */
    static MeshData CreateFullscreenQuad();
};

}  // namespace ENG
