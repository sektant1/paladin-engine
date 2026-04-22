/**
 * @file RenderQueue.h
 * @brief Per-frame command list that batches and issues all draw calls.
 *
 * ## Why a command queue?
 * Separating "what to draw" from "when to draw" gives the engine a clean
 * boundary between game logic and rendering. Application::Update() submits
 * RenderCommands by value; the engine drains and draws the queue once per
 * frame after Update() returns.
 *
 * ## Frame lifecycle
 * 1. Game code (via MeshComponent::Update or manual calls) calls Submit().
 * 2. Engine::Run() calls RenderQueue::Draw() after all updates are done.
 * 3. Draw() iterates the command list, binds each material + mesh, uploads
 *    MVP and light uniforms, and issues the GL draw call.
 * 4. The command list is cleared automatically at the end of Draw(), ready
 *    for the next frame.
 *
 * @see RenderCommand, GraphicsAPI::DrawMesh, Engine::Run
 */

#pragma once

#include <vector>

#include "Common.h"
#include "Types.h"

namespace ENG
{
class Mesh;
class Material;
class GraphicsAPI;

/**
 * @brief One draw call unit: a Mesh + Material + world-space transform.
 *
 * Submitted by application code each frame. The RenderQueue processes it
 * during Draw() by binding the material, uploading the model matrix as a
 * uniform, binding the mesh, and calling GraphicsAPI::DrawMesh().
 */
struct RenderCommand
{
    Mesh     *mesh     = nullptr; ///< Geometry to draw (non-owning raw pointer).
    Material *material = nullptr; ///< Surface definition (non-owning raw pointer).
    mat4      modelMatrix;        ///< Object → World transform for this draw call.
};

/**
 * @brief Accumulates RenderCommands during Update and draws them in one pass.
 *
 * The queue is stateless across frames — it is filled during Update and
 * drained (and cleared) during Draw, forming a clean producer/consumer pattern.
 */
class RenderQueue
{
public:
    /**
     * @brief Add a draw command to the queue for this frame.
     * @param command The mesh + material + transform to be drawn.
     */
    void Submit(const RenderCommand &command);

    /**
     * @brief Draw all submitted commands, then clear the queue.
     *
     * For each command:
     * 1. Bind the material's shader program.
     * 2. Upload view, projection, and model matrices as uniforms.
     * 3. Upload camera position and all light data as uniforms.
     * 4. Bind the material (textures + float params).
     * 5. Bind the mesh VAO and issue the draw call.
     *
     * @param graphicsAPI  The GL wrapper that issues the actual gl* calls.
     * @param cameraData   View and projection matrices for this frame.
     * @param lights       World-space light positions and colours.
     */
    void Draw(GraphicsAPI &graphicsAPI, const CameraData &cameraData, const std::vector<LightData> &lights);

private:
    std::vector<RenderCommand> m_commands;  ///< Commands accumulated this frame; cleared after Draw().
};

}  // namespace ENG
