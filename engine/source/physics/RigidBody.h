/**
 * @file RigidBody.h
 * @ingroup mnd_physics
 * @brief Bullet `btRigidBody` wrapper pairing a @ref Collider with mass and type.
 */

#pragma once
#include <memory>

#include <glm/gtc/quaternion.hpp>

#include "Types.h"
#include "physics/Collider.h"
#include "physics/CollisionObject.h"

class btRigidBody;

namespace mnd
{
/**
 * @ingroup mnd_physics
 * @brief Simulation category for a @ref RigidBody.
 *
 * - **Static**    — never moves; contributes collision only (mass ignored).
 * - **Dynamic**   — fully simulated, driven by forces and gravity.
 * - **Kinematic** — moved by game code; pushes dynamic bodies but is not itself affected.
 */
enum class BodyType
{
    Static,
    Dynamic,
    Kinematic
};

/**
 * @ingroup mnd_physics
 * @brief Owning wrapper around `btRigidBody`; holds a shared @ref Collider,
 *        mass, friction, and body type.
 */
class RigidBody : public CollisionObject
{
 public:
    /**
     * @param type     Simulation category.
     * @param collider Shape used for collision (shared — the same Collider may
     *                 back several bodies).
     * @param mass     Kilograms; ignored for Static bodies.
     * @param friction Bullet friction coefficient in [0, 1].
     */
    RigidBody(BodyType type, const std::shared_ptr<Collider> &collider, float mass, float friction);
    ~RigidBody();

    /// Underlying Bullet body (borrowed, do not delete).
    btRigidBody *GetBody();

    /// Track whether this body is currently registered with a PhysicsManager.
    void SetAddedToWorld(bool added);
    bool IsAddedToWorld() const;

    BodyType GetType() const;

    /// Teleport the body (bypasses the solver — use sparingly on Dynamic bodies).
    void      SetPosition(const glm::vec3 &pos);
    glm::vec3 GetPosition() const;
    void      SetRotation(const glm::quat &rot);
    glm::quat GetRotation() const;

    void ApplyImpulse(const vec3 &impulse);

 private:
    std::unique_ptr<btRigidBody> m_body;
    BodyType                     m_type = BodyType::Static;
    std::shared_ptr<Collider>    m_collider;
    float                        m_mass         = 0.0f;
    float                        m_friction     = 0.5f;
    bool                         m_addedToWorld = false;
};
}  // namespace mnd
