/**
 * @file Common.h
 * @ingroup coa_types
 * @brief Lightweight data-transfer structs shared across subsystems.
 *
 * These plain structs carry per-frame camera and light state from the Scene
 * to the RenderQueue. They are intentionally value types — no ownership,
 * no virtual methods — so they can be cheaply copied into the render pass.
 *
 * Flow:
 *  1. Engine::Run() queries the active Scene for the main camera and lights.
 *  2. The results are packed into CameraData and a vector of LightData.
 *  3. RenderQueue::Draw() receives them and uploads the data as uniforms to
 *     each ShaderProgram before issuing draw calls.
 */

#pragma once

#include "Types.h"
#include "render/Material.h"

namespace COA
{

/**
 * @brief Per-frame camera matrices and world-space position.
 *
 * Produced by CameraComponent each frame and consumed by RenderQueue::Draw()
 * to set the view/projection uniforms on every material that is drawn.
 */
struct CameraData
{
    mat4 viewMatrix;        ///< World → Camera space transform (glm::lookAt result).
    mat4 projectionMatrix;  ///< Camera → Clip space transform (perspective or ortho).
    vec3 position;          ///< Camera world-space position, used for specular calculations.
};

/**
 * @brief World-space position and RGB colour of a single point light.
 *
 * Collected by Scene::CollectLight() from all GameObjects that carry a
 * LightComponent, then forwarded to RenderQueue::Draw() to set light uniforms.
 */
struct LightData
{
    vec3 color;     ///< Linear RGB light colour (1,1,1 = white full-intensity).
    vec3 position;  ///< World-space origin of the light source.
};

}  // namespace COA
