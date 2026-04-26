#pragma once

#include "Monad.h"

namespace mnd
{
class AnimationComponent;
class AudioComponent;
class PlayerControllerComponent;
}  // namespace mnd

class Player : public mnd::GameObject
{
    GAMEOBJECT(Player)
public:
    Player() = default;
    void Init();
    void Update(mnd::f32 deltaTime) override;

private:
    void SetViewmodel(bool useHands);
    void RegisterHandsTweakerPanel();
    void RebuildIdleClip();

    mnd::AnimationComponent        *m_animationComponent        = nullptr;  ///< Gun anim (legacy carbine).
    mnd::AnimationComponent        *m_handsAnimComponent        = nullptr;  ///< Arms idle/anim driver.
    mnd::AudioComponent            *m_audioComponent            = nullptr;
    mnd::PlayerControllerComponent *m_playerControllerComponent = nullptr;

    mnd::GameObject *m_gunObject   = nullptr;
    mnd::GameObject *m_handsObject = nullptr;

    bool m_usingHands = false;  ///< false = gun (default, matches legacy behavior).

    // F1 tweaker state — surfaced through the editor's "Hands Tweaker" panel.
    mnd::vec3 m_handsPosition  = mnd::vec3(0.0f, -0.4f, -0.6f);
    mnd::vec3 m_handsRotEuler  = mnd::vec3(0.0f);                   ///< Degrees, intrinsic XYZ.
    mnd::f32  m_handsScale     = 0.01f;                             ///< Uniform scale.
    mnd::f32  m_idleDuration   = 4.0f;
    mnd::f32  m_idleAmpScale   = 1.0f;
};
