/**
 * @file Mesh.h
 * @ingroup coa_render
 * @brief GPU mesh: Vertex Array Object wrapping a VBO and optional EBO.
 *
 * A Mesh holds the GPU-side geometry for a drawable object. It is created by
 * Builder factory methods (or Mesh::CreateCube) and stored in a shared_ptr so
 * that multiple MeshComponents can share the same geometry data.
 *
 * ## Relationship with Material
 * Mesh and Material are intentionally decoupled — they are paired only at
 * submit time inside a RenderCommand. This allows one mesh to be drawn with
 * different materials (e.g. a highlight variant) without duplication.
 *
 * ## Draw call flow
 * @code
 *   graphicsAPI.BindMesh(mesh);       // glBindVertexArray → sets up attrib pointers
 *   mesh->Draw();                     // glDrawElements or glDrawArrays
 * @endcode
 *
 * @see Builder, MeshComponent, RenderCommand
 */

#pragma once
#include <memory>
#include <vector>

#include "GL/glew.h"
#include "Types.h"
#include "graphics/VertexLayout.h"

namespace COA
{

/**
 * @brief Immutable GPU mesh (VAO + VBO + optional EBO).
 *
 * Non-copyable — each instance owns unique GL handles.
 * Vertex data is uploaded to the GPU in the constructor and released from
 * CPU memory immediately; the GL objects live until the Mesh is destroyed.
 */
class Mesh
{
public:
    /**
     * @brief Upload indexed geometry to the GPU.
     * @param layout   Describes how vertex bytes map to shader attributes.
     * @param vertices Interleaved vertex data (position, normal, UV …).
     * @param indices  Triangle list indices into the vertex array.
     */
    Mesh(const VertexLayout &layout, const std::vector<float> &vertices, const std::vector<uint32_t> &indices);

    /**
     * @brief Upload non-indexed geometry to the GPU.
     * @param layout   Vertex attribute layout.
     * @param vertices Interleaved vertex data; drawn with glDrawArrays.
     */
    Mesh(const VertexLayout &layout, const std::vector<float> &vertices);

    Mesh(const Mesh &)            = delete;
    Mesh &operator=(const Mesh &) = delete;

    /// Bind the VAO so subsequent draw calls use this mesh's geometry.
    void Bind();

    /**
     * @brief Issue the draw call for this mesh.
     *
     * Uses glDrawElements when an EBO is present; glDrawArrays otherwise.
     * Must be preceded by Bind() (or GraphicsAPI::BindMesh()).
     */
    void Draw();

    /// Creates a unit cube mesh (factory convenience, same as Builder::CreateCube).
    static std::shared_ptr<Mesh> CreateCube();

private:
    VertexLayout m_vertexLayout;      ///< Attribute layout used when setting up the VAO.

    GLuint m_VBO         = 0;  ///< Vertex Buffer Object — raw vertex data on the GPU.
    GLuint m_EBO         = 0;  ///< Element Buffer Object — index data (0 if non-indexed).
    GLuint m_VAO         = 0;  ///< Vertex Array Object — records attribute layout + buffer bindings.
    size_t m_vertexCount = 0;  ///< Number of vertices (used for glDrawArrays).
    size_t m_indexCount  = 0;  ///< Number of indices (used for glDrawElements; 0 if non-indexed).
};

}  // namespace COA
