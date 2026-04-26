/**
 * @file PlayerControllerComponent.h
 * @ingroup mnd_components
 * @brief First-person Quake/Half-Life-style movement controller.
 *
 * Implements ground friction, ground/air acceleration with a per-axis wishspeed
 * cap (the trick that enables strafe-jumping and bunny-hopping), and an
 * optional auto-hop while the jump key is held.
 *
 * Attach to a GameObject that also has a CameraComponent. Each frame Update()
 * reads keyboard (WASD + Space) and mouse-delta input from the Engine's
 * InputManager and drives a KinematicCharacterController.
 *
 * @see InputManager, CameraComponent, KinematicCharacterController
 */

#pragma once

#include <memory>

#include "Constants.h"
#include "Types.h"
#include "physics/KinematicCharacterController.h"
#include "scene/Component.h"

namespace mnd
{

class PlayerControllerComponent : public Component
{
    COMPONENT(PlayerControllerComponent)
public:
    void Init() override;
    void Update(f32 deltaTime) override;

    [[nodiscard]] bool OnGround() const;

    void              SetMoveSpeed(f32 speed);
    [[nodiscard]] f32 GetMoveSpeed() const;

    [[nodiscard]] f32 GetSensitivity() const { return m_sensitivity; }
    [[nodiscard]] f32 GetJumpSpeed() const { return m_jumpSpeed; }

    void SetSensitivity(f32 s) { m_sensitivity = s; }
    void SetJumpSpeed(f32 s) { m_jumpSpeed = s; }

    // Quake tunables --------------------------------------------------------
    [[nodiscard]] f32 GetGroundAccel() const { return m_groundAccel; }
    [[nodiscard]] f32 GetAirAccel() const { return m_airAccel; }
    [[nodiscard]] f32 GetFriction() const { return m_friction; }
    [[nodiscard]] f32 GetAirCap() const { return m_airCap; }
    [[nodiscard]] bool GetAutoHop() const { return m_autoHop; }

    void SetGroundAccel(f32 v) { m_groundAccel = v; }
    void SetAirAccel(f32 v) { m_airAccel = v; }
    void SetFriction(f32 v) { m_friction = v; }
    void SetAirCap(f32 v) { m_airCap = v; }
    void SetAutoHop(bool v) { m_autoHop = v; }

private:
    /// Quake PM_Friction: shrink horizontal velocity each frame on ground.
    void ApplyFriction(f32 deltaTime);
    /// Quake PM_Accelerate: project velocity onto wishdir, add up to wishspeed.
    void Accelerate(const vec3 &wishDir, f32 wishSpeed, f32 accel, f32 deltaTime);

    f32 m_sensitivity = kDefaultMouseSensitivity;
    f32 m_moveSpeed   = kDefaultMoveSpeed;
    f32 m_jumpSpeed   = kDefaultJumpSpeed;
    f32 m_yRot        = 0.0F;
    f32 m_xRot        = 0.0F;

    f32  m_groundAccel = kQuakeGroundAccel;
    f32  m_airAccel    = kQuakeAirAccel;
    f32  m_friction    = kQuakeFriction;
    f32  m_airCap      = kQuakeAirCap;
    bool m_autoHop     = true;  ///< Auto-rejump while Space is held (bhop).

    vec3 m_velocity      = vec3(0.0F);  ///< Tracked horizontal velocity (XZ; Y unused).
    bool m_jumpHeldLast  = false;       ///< Edge-detect Space release for non-auto-hop.

    std::unique_ptr<KinematicCharacterController> m_kinematicController;
};

}  // namespace mnd
