#pragma once 

#include "scene/Component.h"
#include "physics/KinematicCharacterController.h"
#include <memory>

namespace eng
{
    class PlayerControllerComponent : public Component
    {
        COMPONENT(PlayerControllerComponent)

    public:
        void Init() override;
        void Update(float deltaTime) override;
        bool OnGround() const;

    private:
        float m_sensitivity = 15.0f;
        float m_moveSpeed = 50.0f;
        float m_xRot = 0.0f;
        float m_yRot = 0.0f;
        std::unique_ptr<KinematicCharacterController> m_kinematicController;
    };
}