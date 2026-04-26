#pragma once

#include <Monad.h>

class JumpPlatform
    : public mnd::GameObject
    , public mnd::IContactListener
{
    GAMEOBJECT(JumpPlatform)
 public:
    void Init() override;
    void OnContact(mnd::CollisionObject *obj, const mnd::vec3 &pos, const mnd::vec3 &norm) override;
};

