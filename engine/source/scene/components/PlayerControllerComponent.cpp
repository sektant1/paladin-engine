#include "scene/components/PlayerControllerComponent.h"

#include "Constants.h"
#include "Engine.h"
#include "GLFW/glfw3.h"
#include "Types.h"
#include "editor/Editor.h"
#include "input/InputManager.h"

namespace COA
{

void PlayerControllerComponent::Init()
{
    m_kinematicController = std::make_unique<KinematicCharacterController>(
        kDefaultCapsuleRadius, kDefaultCapsuleHeight, m_owner->GetWorldPosition());
}

void PlayerControllerComponent::Update(f32 deltaTime)
{
    auto &inputManager = Engine::GetInstance().GetInputManager();
    const bool editorOpen = Engine::GetInstance().GetEditor().IsVisible();

    auto rotation = m_owner->GetRotation();

    if (!editorOpen && inputManager.IsMousePositionChanged())
    {
        const auto &oldPos     = inputManager.GetMousePositionOld();
        const auto &currentPos = inputManager.GetMousePositionCurrent();

        f32 deltaX = currentPos.x - oldPos.x;
        f32 deltaY = currentPos.y - oldPos.y;

        // Yaw
        float yDeltaAngle = -deltaX * m_sensitivity * deltaTime;
        m_yRot += yDeltaAngle;
        glm::quat yRot = glm::angleAxis(glm::radians(m_yRot), glm::vec3(0, 1, 0));

        // Pitch
        float xDeltaAngle = -deltaY * m_sensitivity * deltaTime;
        m_xRot += xDeltaAngle;
        m_xRot         = std::clamp(m_xRot, -kPitchLimitDegrees, kPitchLimitDegrees);
        glm::quat xRot = glm::angleAxis(glm::radians(m_xRot), glm::vec3(1, 0, 0));

        // Final rotation
        rotation = glm::normalize(yRot * xRot);
        m_owner->SetRotation(rotation);
    }

    vec3 front = rotation * vec3(0.0f, 0.0f, -1.0f);
    vec3 right = rotation * vec3(1.0f, 0.0f, 0.0f);

    auto position = m_owner->GetPosition();

    vec3 move(0.0f);
    if (editorOpen)
    {
        m_kinematicController->Walk(vec3(0.0f));
        m_owner->SetPosition(m_kinematicController->GetPosition());
        return;
    }
    if (inputManager.IsKeyPressed(Key::A))
    {
        move -= right;
    } else if (inputManager.IsKeyPressed(Key::D))
    {
        move += right;
    }
    if (inputManager.IsKeyPressed(Key::W))
    {
        move += front;
    } else if (inputManager.IsKeyPressed(Key::S))
    {
        move -= front;
    }

    if (glm::dot(move, move) > 0)
    {
        move = glm::normalize(move);
    }
    m_kinematicController->Walk(move * m_moveSpeed * deltaTime);

    if (inputManager.IsKeyPressed(Key::Space))
    {
        m_kinematicController->Jump(vec3(0.0f, m_moveSpeed * m_jumpSpeed, 0.0f));
    }

    // Sync scene object with physics controller
    m_owner->SetPosition(m_kinematicController->GetPosition());
}

bool PlayerControllerComponent::OnGround() const
{
    if (m_kinematicController)
    {
        return m_kinematicController->OnGround();
    }
    return false;
}

void PlayerControllerComponent::SetMoveSpeed(f32 speed)
{
    m_moveSpeed = speed;
}

f32 PlayerControllerComponent::GetMoveSpeed() const
{
    return m_moveSpeed;
}

}  // namespace COA
