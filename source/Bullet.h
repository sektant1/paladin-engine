#pragma once

#include "COA.h"
#include "GameConstants.h"

class Bullet : public COA::GameObject
{
    GAMEOBJECT(Bullet)

public:
    void Update(COA::f32 deltaTime) override;

private:
    COA::f32 m_lifetime = kBulletLifetime;
};
