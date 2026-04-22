/**
 * @file VertexLayout.h
 * @brief Describes the memory layout of a single vertex in a VBO.
 *
 * A VertexLayout is a list of VertexElements — each element maps one
 * interleaved field (position, UV, normal …) to a numbered shader attribute.
 *
 * The layout is created by Builder factory methods together with the vertex
 * data and is then stored inside Mesh. When Mesh::Bind() is called,
 * GraphicsAPI reads the layout to issue the matching glVertexAttribPointer
 * calls, connecting the raw buffer bytes to the GLSL attribute slots.
 *
 * Example interleaved vertex (pos + normal + UV) looks like:
 * @code
 *  | x  y  z | nx ny nz | u  v |   ← one vertex, stride = 8 floats
 *  ^--- offset 0 ---^--- offset 12 ---^--- offset 24
 * @endcode
 *
 * @see Builder, Mesh, GraphicsAPI::BindMesh
 */

#pragma once

#include <cstdint>
#include <vector>

#include <GL/glew.h>

namespace ENG
{

/**
 * @brief Describes one interleaved vertex attribute within a VBO.
 *
 * Maps directly to a glVertexAttribPointer call:
 * - `index`  → attrib location in the vertex shader.
 * - `size`   → component count (1–4).
 * - `type`   → GL data type (typically GL_FLOAT).
 * - `offset` → byte offset from the start of the vertex struct.
 */
struct VertexElement
{
    GLuint   index;   ///< Shader attribute location (layout(location = N)).
    GLuint   size;    ///< Number of components (e.g. 3 for a vec3 position).
    GLuint   type;    ///< GL data type constant (e.g. GL_FLOAT).
    uint32_t offset;  ///< Byte offset from the start of one vertex.

    /// @brief Canonical attribute slot indices — must match the GLSL layout locations.
    /// @{
    static constexpr int PositionIndex = 0;  ///< layout(location = 0) vec3 aPosition
    static constexpr int ColorIndex    = 1;  ///< layout(location = 1) vec3 aColor
    static constexpr int UVIndex       = 2;  ///< layout(location = 2) vec2 aUV
    static constexpr int NormalIndex   = 3;  ///< layout(location = 3) vec3 aNormal
    /// @}
};

/**
 * @brief Complete description of how vertex data is packed in a VBO.
 *
 * Holds all VertexElements for one mesh type plus the total stride (bytes
 * between the start of consecutive vertices). Created by Builder factory
 * functions and stored inside Mesh until the mesh is destroyed.
 */
struct VertexLayout
{
    std::vector<VertexElement> elements;     ///< Ordered list of vertex attributes.
    uint32_t                   stride = 0;   ///< Total byte size of one vertex.
};

}  // namespace ENG
