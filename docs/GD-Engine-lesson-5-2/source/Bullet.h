#pragma once

#include <eng.h>

class Bullet : public eng::GameObject
{
    GAMEOBJECT(Bullet)
public:
    void Update(float deltaTime) override;

private:
    float m_lifetime = 2.0f;
};