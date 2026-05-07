#pragma once

#include "GameConstants.h"
#include "Monad.h"

class Bullet
    : public mnd::GameObject
    , public mnd::IContactListener
{
    GAMEOBJECT(Bullet)

 public:
    ~Bullet() override;

    void Update(mnd::f32 deltaTime) override;

    void OnContact(mnd::CollisionObject *obj, const mnd::vec3 &pos, const mnd::vec3 &norm) override;

    void SetShooter(mnd::GameObject *shooter) { m_shooter = shooter; }
    void SetDamage(float damage) { m_damage = damage; }

 private:
    void TryHookContactListener();

    mnd::f32         m_lifetime       = kBulletLifetime;
    mnd::GameObject *m_shooter        = nullptr;
    float            m_damage         = 25.0F;
    bool             m_listenerHooked = false;
    bool             m_consumed       = false;
};
