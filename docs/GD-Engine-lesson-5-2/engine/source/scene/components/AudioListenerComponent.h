#pragma once
#include "scene/Component.h"

namespace eng
{
    class AudioListenerComponent : public Component
    {
        COMPONENT(AudioListenerComponent)
    public:
        void Update(float deltaTime) override;
    };
}