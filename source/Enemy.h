#pragma once

#include <memory>
#include <string>

#include "Monad.h"

class Enemy : public mnd::GameObject
{
    GAMEOBJECT(Enemy)

 public:
    void LoadProperties(const nlohmann::json &json) override;
    void Init() override;
    void Update(mnd::f32 deltaTime) override;

 private:
    mnd::GameObject *FindTarget() const;
    void             EnsureHealth();
    void             SpawnDeathParticles();
    void             EnsurePhysics();
    void             CreateCapsuleVisual();
    mnd::vec3        ResolveAxisMove(const mnd::vec3 &delta) const;
    bool             AxisBlocked(int axis, mnd::f32 step) const;

    mnd::vec3   m_visualOffset{0.0F};
    mnd::f32    m_capsuleRadius = 0.55F;
    mnd::f32    m_capsuleHeight = 2.0F;
    mnd::vec3   m_colliderHalfExtents{0.55F, 1.0F, 0.55F};
    int         m_maxHealth     = 75;
    mnd::f32    m_moveSpeed     = 1.4F;
    mnd::f32    m_detectionRange = 18.0F;
    mnd::f32    m_stopDistance  = 1.7F;

    std::shared_ptr<mnd::RigidBody> m_rigidBody;
    mnd::GameObject                *m_visualRoot = nullptr;
};
