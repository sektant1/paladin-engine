/**
 * @file CameraComponent.h
 * @brief Perspective camera that drives the scene's view and projection matrices.
 *
 * Attach this component to a GameObject and call Scene::SetMainCamera() on
 * that object. Engine::Run() queries it each frame to compute CameraData
 * (view + projection) which is then forwarded to every shader via RenderQueue.
 *
 * The view matrix is built from the owner's world transform (position and
 * rotation). The projection matrix is perspective, driven by m_fov and the
 * window's current aspect ratio (passed in by the engine each frame).
 *
 * @see CameraData, Scene::SetMainCamera, Engine::Run
 */

#pragma once

#include "Types.h"
#include "scene/Component.h"

namespace ENG
{

/**
 * @brief Perspective camera — provides view and projection matrices each frame.
 *
 * Works in conjunction with the owning GameObject's transform:
 * - Position and rotation of the owner define where the camera is and where it looks.
 * - m_fov, m_nearPlane, m_farPlane define the viewing frustum.
 */
class CameraComponent : public Component
{
    COMPONENT(CameraComponent)
public:
    /// No per-frame logic needed; view/projection are computed on demand.
    void Update(f32 deltaTime) override;

    /**
     * @brief Compute the view matrix from the owner's world transform.
     *
     * Equivalent to glm::inverse(owner->GetWorldTransform()). Converts
     * world-space coordinates into camera-space (eye space).
     *
     * @return Column-major 4×4 view matrix.
     */
    [[nodiscard]] mat4 GetViewMatrix() const;

    /**
     * @brief Compute the perspective projection matrix.
     *
     * Maps camera-space coordinates to clip space using glm::perspective.
     *
     * @param aspect Window width / height (updated each frame by the engine).
     * @return Column-major 4×4 projection matrix.
     */
    [[nodiscard]] mat4 GetProjectionMatrix(f32 aspect) const;

private:
    f32 m_fov       = 60.0F;    ///< Vertical field of view in degrees.
    f32 m_nearPlane = 0.1F;     ///< Distance to the near clip plane (avoid setting to 0).
    f32 m_farPlane  = 1000.0F;  ///< Distance to the far clip plane.
};

}  // namespace ENG
