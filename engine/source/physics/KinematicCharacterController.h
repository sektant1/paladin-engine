/**
 * @file KinematicCharacterController.h
 * @ingroup coa_physics
 * @brief Capsule-shaped character controller backed by Bullet's
 *        `btKinematicCharacterController`.
 */

#pragma once
#include <memory>

#include "Constants.h"
#include "Types.h"

class btPairCachingGhostObject;
class btKinematicCharacterController;

namespace COA
{

/**
 * @ingroup coa_physics
 * @brief High-level capsule character controller: walks, jumps, detects ground.
 *
 * Unlike a @ref RigidBody, this object is **not** force-driven. Game code calls
 * @ref Walk / @ref Jump with desired directions and Bullet resolves the resulting
 * motion against the collision world.
 */
class KinematicCharacterController
{
public:
    /**
     * @param radius   Capsule radius (metres).
     * @param height   Capsule height (metres, full — caps included).
     * @param position Initial world position.
     */
    KinematicCharacterController(float radius, float height, const glm::vec3 &position);
    ~KinematicCharacterController();

    [[nodiscard]] vec3 GetPosition() const;
    [[nodiscard]] quat GetRotation() const;

    /// Apply a walk velocity in world space for this frame.
    void Walk(const vec3 &direction);

    /// Apply an impulse-style jump in @p direction (typically `{0, jumpSpeed, 0}`).
    void Jump(const vec3 &direction);

    /// True when the controller has a contact point beneath it.
    [[nodiscard]] bool OnGround() const;

private:
    float m_height = kDefaultCapsuleHeight;
    float m_radius = kDefaultCapsuleRadius;

    std::unique_ptr<btPairCachingGhostObject>       m_ghost;
    std::unique_ptr<btKinematicCharacterController> m_controller;
};

}  // namespace COA
