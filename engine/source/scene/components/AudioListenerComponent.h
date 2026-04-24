#pragma once
#include "scene/Component.h"

namespace COA
{
class AudioListenerComponent : public Component
{
    COMPONENT(AudioListenerComponent)
public:
    void Update(float deltaTime) override;
};
}  // namespace COA

