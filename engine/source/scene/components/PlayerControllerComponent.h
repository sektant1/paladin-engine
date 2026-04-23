/**
 * @file PlayerControllerComponent.h
 * @ingroup coa_components
 * @brief First-person camera and movement controller driven by InputManager.
 *
 * Attach to a GameObject that also has a CameraComponent. Each frame Update()
 * reads keyboard (WASD) and mouse-delta input from the Engine's InputManager
 * and moves / rotates the owner accordingly.
 *
 * @see InputManager, CameraComponent
 */

#pragma once

#include "Types.h"
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
    void Update(f32 deltaTime) override;

private:
    f32 m_sensitivity = 1.5F;  ///< Mouse-look sensitivity multiplier (degrees per pixel).
    f32 m_moveSpeed   = 1.0F;  ///< Translation speed in world units per second.
};

}  // namespace COA
