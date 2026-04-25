/**
 * @file CameraComponent.h
 * @ingroup coa_components
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

#include "Constants.h"
#include "Types.h"
#include "scene/Component.h"

namespace COA
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

    [[nodiscard]] f32 GetFov() const { return m_fov; }

    [[nodiscard]] f32 GetNearPlane() const { return m_nearPlane; }

    [[nodiscard]] f32 GetFarPlane() const { return m_farPlane; }

    void SetFov(f32 fov) { m_fov = fov; }

    void SetNearPlane(f32 n) { m_nearPlane = n; }

    void SetFarPlane(f32 f) { m_farPlane = f; }

private:
    f32 m_fov       = kCameraFov;        ///< Vertical field of view in degrees.
    f32 m_nearPlane = kCameraNearPlane;  ///< Distance to the near clip plane (avoid setting to 0).
    f32 m_farPlane  = kCameraFarPlane;   ///< Distance to the far clip plane.
};

}  // namespace COA
