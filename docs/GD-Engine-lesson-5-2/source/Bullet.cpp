#include "Bullet.h"

void Bullet::Update(float deltaTime)
{
    eng::GameObject::Update(deltaTime);
    m_lifetime -= deltaTime;
    if (m_lifetime <= 0.0f)
    {
        MarkForDestroy();
    }
}