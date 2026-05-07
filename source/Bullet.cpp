#include "Bullet.h"

#include "combat/DamageEvent.h"
#include "combat/DamageSystem.h"

Bullet::~Bullet()
{
    auto *physics = GetComponent<mnd::PhysicsComponent>();
    if (physics == nullptr)
    {
        return;
    }

    const auto &rigidBody = physics->GetRigidBody();
    if (rigidBody)
    {
        rigidBody->RemoveContactListener(this);
    }
}

void Bullet::Update(mnd::f32 deltaTime)
{
    mnd::GameObject::Update(deltaTime);

    TryHookContactListener();

    if (m_consumed)
    {
        MarkForDestroy();
        return;
    }

    m_lifetime -= deltaTime;
    if (m_lifetime <= 0.0F)
    {
        MarkForDestroy();
    }
}

void Bullet::TryHookContactListener()
{
    if (m_listenerHooked)
    {
        return;
    }

    auto *physics = GetComponent<mnd::PhysicsComponent>();
    if (physics == nullptr)
    {
        return;
    }

    const auto &rigidBody = physics->GetRigidBody();
    if (!rigidBody)
    {
        return;
    }

    rigidBody->AddContactListener(this);
    m_listenerHooked = true;
}

void Bullet::OnContact(mnd::CollisionObject *obj, const mnd::vec3 &pos, const mnd::vec3 &norm)
{
    if (m_consumed || obj == nullptr)
    {
        return;
    }

    auto *victim = obj->GetOwner();
    if (victim == m_shooter)
    {
        // Bullet collided with its own shooter on the spawn frame — ignore.
        return;
    }

    if (victim != nullptr && victim->GetComponent<mnd::HealthComponent>() != nullptr)
    {
        DamageEvent evt;
        evt.attacker  = m_shooter;
        evt.victim    = victim;
        evt.amount    = m_damage;
        evt.type      = DamageType::Bullet;
        evt.hitPoint  = pos;
        evt.hitNormal = norm;
        DamageSystem::Apply(evt);
    }

    // Defer destruction to Update — destroying mid-physics-dispatch invalidates
    // the rigid body whose contact callback we're still inside.
    m_consumed = true;
}
