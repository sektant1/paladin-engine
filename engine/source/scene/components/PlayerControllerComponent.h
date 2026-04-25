/**
 * @file PlayerControllerComponent.h
 * @ingroup coa_components
 * @brief First-person camera and movement controller driven by InputManager.
 *
 * Attach to a GameObject that also has a CameraComponent. Each frame Update()
 * reads keyboard (WASD) and mouse-delta input from the Engine's InputManager
 * and moves / rotates the owner accordingly.
 * @see InputManager, CameraComponent */

#pragma once

#include <memory>

#include "Constants.h"
#include "Types.h"
#include "physics/KinematicCharacterController.h"
#include "scene/Component.h"

namespace COA
{

/**
 * @brief First-person movement and mouse-look controller.
 *
 * Reads WASD keys for translation and mouse delta for yaw/pitch rotation.
 * m_sensitivity scales the angular response to mouse movement;
 * m_moveSpeed scales the translation distance per second.
 */
class PlayerControllerComponent : public Component
{
    COMPONENT(PlayerControllerComponent)
public:
    /**
     * @brief Process input and update the owner's transform.
     * @param deltaTime Seconds elapsed since the last frame.
     */
    void Init() override;

    void Update(f32 deltaTime) override;

    [[nodiscard]] bool OnGround() const;

    void SetMoveSpeed(f32 speed);
    f32  GetMoveSpeed() const;

    [[nodiscard]] f32 GetSensitivity() const { return m_sensitivity; }
    [[nodiscard]] f32 GetJumpSpeed() const { return m_jumpSpeed; }

    void SetSensitivity(f32 s) { m_sensitivity = s; }
    void SetJumpSpeed(f32 s) { m_jumpSpeed = s; }

private:
    f32 m_sensitivity = kDefaultMouseSensitivity;  ///< Mouse-look sensitivity multiplier (degrees per pixel).
    f32 m_moveSpeed   = kDefaultMoveSpeed;         ///< Translation speed in world units per second.
    f32 m_jumpSpeed   = kDefaultJumpSpeed;         ///< Multiplier on m_moveSpeed for the jump impulse.
    f32 m_yRot        = 0.0F;
    f32 m_xRot        = 0.0F;

    std::unique_ptr<KinematicCharacterController> m_kinematicController;
};

}  // namespace COA
