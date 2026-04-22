/**
 * @file GraphicsAPI.h
 * @brief Thin wrapper around raw OpenGL 3.3 Core Profile calls.
 *
 * GraphicsAPI sits between the application and GLEW/GL. It creates and
 * manages GPU resources (VBOs, IBOs, shader programs) and issues draw calls.
 * All other engine code — particularly RenderQueue — goes through this class
 * rather than calling gl* functions directly, making the rendering layer easy
 * to swap out or extend without touching game code.
 *
 * Owned by the Engine singleton; access via Engine::GetInstance().GetGraphicsAPI().
 *
 * @see RenderQueue, ShaderProgram, Mesh, Material
 */

#pragma once
#include <memory>
#include <string>
#include <vector>

#include "GL/glew.h"

namespace ENG
{

class ShaderProgram;
class Material;
class Mesh;

/**
 * @brief OpenGL 3.3 Core Profile abstraction layer.
 *
 * Responsibilities:
 * - Initialise default render state (depth test, blending …).
 * - Compile and link GLSL programs via CreateShaderProgram().
 * - Allocate VBO / IBO objects on the GPU.
 * - Bind materials and meshes to the current GL state before draw calls.
 */
class GraphicsAPI
{
public:
    /**
     * @brief Set up initial OpenGL state (depth test, face culling, etc.).
     * @return true on success.
     */
    bool Init();

    /**
     * @brief Compile vertex + fragment GLSL source and link into a program.
     * @param vertexSource   GLSL source string for the vertex stage.
     * @param fragmentSource GLSL source string for the fragment stage.
     * @return Shared pointer to the linked ShaderProgram.
     */
    std::shared_ptr<ShaderProgram> CreateShaderProgram(const std::string &vertexSource,
                                                       const std::string &fragmentSource);

    /**
     * @brief Returns the built-in default shader (Blinn-Phong + diffuse texture).
     *
     * Loaded once during Init(). Materials that do not specify their own
     * shader fall back to this program.
     */
    const std::shared_ptr<ShaderProgram> &GetDefaultShaderProgram();

    /**
     * @brief Upload a flat array of floats to a new GPU vertex buffer.
     * @param vertices Interleaved vertex data (position, normal, UV, …).
     * @return GL handle to the created VBO.
     */
    GLuint CreateVertexBuffer(const std::vector<float> &vertices);

    /**
     * @brief Upload a flat array of u32 indices to a new GPU index buffer.
     * @param indices Triangle list indices into the vertex buffer.
     * @return GL handle to the created EBO.
     */
    GLuint CreateIndexBuffer(const std::vector<uint32_t> &indices);

    /**
     * @brief Set the colour that glClear() fills the framebuffer with.
     * @param r,g,b,a Linear RGBA components in [0, 1].
     */
    void SetClearColor(float r, float g, float b, float a);

    /// Clear the colour and depth buffers ready for the next frame.
    void ClearBuffers();

    /**
     * @brief Bind a compiled shader program to the current GL state.
     * @param shaderProgram The program to activate; must not be nullptr.
     */
    void BindShaderProgram(ShaderProgram *shaderProgram);

    /**
     * @brief Upload all material parameters as uniforms to the active shader.
     * @param material Material whose float / texture params are applied.
     */
    void BindMaterial(Material *material);

    /**
     * @brief Bind the mesh's VAO so the next draw call uses its geometry.
     * @param mesh Mesh whose VAO/VBO/EBO layout is activated.
     */
    void BindMesh(Mesh *mesh);

    /**
     * @brief Issue a draw call for the bound mesh.
     *
     * Uses indexed drawing (glDrawElements) when the mesh has an EBO,
     * otherwise falls back to glDrawArrays.
     *
     * @param mesh The mesh to draw (must already be bound via BindMesh).
     */
    void DrawMesh(Mesh *mesh);

private:
    /// Fallback shader compiled on Init(); used when no material provides one.
    std::shared_ptr<ShaderProgram> m_defaultShaderProgram;
};

}  // namespace ENG
