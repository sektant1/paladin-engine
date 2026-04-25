#pragma once

#include "COA.h"

namespace COA
{
class AnimationComponent;
class AudioComponent;
class PlayerControllerComponent;
}  // namespace COA

class Player : public COA::GameObject
{
    GAMEOBJECT(Player)
public:
    Player() = default;
    void Init();
    void Update(COA::f32 deltaTime) override;

private:
    COA::AnimationComponent        *m_animationComponent        = nullptr;
    COA::AudioComponent            *m_audioComponent            = nullptr;
    COA::PlayerControllerComponent *m_playerControllerComponent = nullptr;
};
