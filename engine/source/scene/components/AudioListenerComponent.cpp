#include "scene/components/AudioListenerComponent.h"

#include "Engine.h"
#include "scene/GameObject.h"

namespace COA
{
void AudioListenerComponent::Update(float deltaTime)
{
    auto pos = m_owner->GetWorldPosition();
    Engine::GetInstance().GetAudioManager().SetListenerPosition(pos);
}
}  // namespace COA

