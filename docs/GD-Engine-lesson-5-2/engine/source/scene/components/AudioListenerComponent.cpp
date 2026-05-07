#include "scene/components/AudioListenerComponent.h"
#include "scene/GameObject.h"
#include "Engine.h"

namespace eng
{
    void AudioListenerComponent::Update(float deltaTime)
    {
        auto pos = m_owner->GetWorldPosition();
        Engine::GetInstance().GetAudioManager().SetListenerPosition(pos);
    }
}