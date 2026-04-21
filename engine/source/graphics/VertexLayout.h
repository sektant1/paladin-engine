#pragma once

#include <cstdint>
#include <vector>

#include <GL/glew.h>

namespace ENG
{

struct VertexElement
{
    GLuint   index;   // Attribute location
    GLuint   size;    // Number of components
    GLuint   type;    // Data type (e.g. GL_FLOAT)
    uint32_t offset;  // Bytes offset from start of vertex

    static constexpr int PositionIndex = 0;
    static constexpr int ColorIndex    = 1;
    static constexpr int UVIndex       = 2;
};

struct VertexLayout
{
    std::vector<VertexElement> elements;
    uint32_t                   stride = 0;  // Total size of a single vertex
};

}  // namespace ENG
