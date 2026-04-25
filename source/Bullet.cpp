#include "Bullet.h"

void Bullet::Update(COA::f32 deltaTime)
{
    COA::GameObject::Update(deltaTime);
    m_lifetime -= deltaTime;
    if (m_lifetime <= 0.0F)
    {
        MarkForDestroy();
    }
}
