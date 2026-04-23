#include "scene/components/PlayerControllerComponent.h"

#include "Engine.h"
#include "GLFW/glfw3.h"
#include "Types.h"
#include "input/InputManager.h"

namespace COA
{

void PlayerControllerComponent::Update(f32 deltaTime)
{
    auto &inputManager = Engine::GetInstance().GetInputManager();

    auto rotation = m_owner->GetRotation();

    if (inputManager.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
        const auto &oldPos     = inputManager.GetMousePositionOld();
        const auto &currentPos = inputManager.GetMousePositionCurrent();

        f32 deltaX = currentPos.x - oldPos.x;
        f32 deltaY = currentPos.y - oldPos.y;

        // rotation.y -= deltaX * m_sensitivity * deltaTime;
        // rotation.x -= deltaY * m_sensitivity * deltaTime;

        f32  yAngle = -deltaX * m_sensitivity * deltaTime;
        quat yRot   = angleAxis(yAngle, vec3(0.0f, 1.0f, 0.0f));

        f32  xAngle = -deltaY * m_sensitivity * deltaTime;
        vec3 right  = rotation * vec3(1.0f, 0.0f, 0.0f);
        quat xRot   = angleAxis(xAngle, right);

        quat deltaRot = yRot * xRot;
        rotation      = normalize(deltaRot * rotation);

        m_owner->SetRotation(rotation);
    }

    vec3 front = rotation * vec3(0.0f, 0.0f, -1.0f);
    vec3 right = rotation * vec3(1.0f, 0.0f, 0.0f);

    auto position = m_owner->GetPosition();

    if (inputManager.IsKeyPressed(GLFW_KEY_W)) {
        position += front * m_moveSpeed * deltaTime;
    }
    if (inputManager.IsKeyPressed(GLFW_KEY_A)) {
        position -= right * m_moveSpeed * deltaTime;
    }
    if (inputManager.IsKeyPressed(GLFW_KEY_S)) {
        position -= front * m_moveSpeed * deltaTime;
    }
    if (inputManager.IsKeyPressed(GLFW_KEY_D)) {
        position += right * m_moveSpeed * deltaTime;
    }
    // Vertical noclip: world-up axis, independent of pitch -> predictable flight.
    if (inputManager.IsKeyPressed(GLFW_KEY_SPACE)) {
        position += vec3(0.0F, 1.0F, 0.0F) * m_moveSpeed * deltaTime;
    }
    if (inputManager.IsKeyPressed(GLFW_KEY_LEFT_CONTROL)) {
        position -= vec3(0.0F, 1.0F, 0.0F) * m_moveSpeed * deltaTime;
    }
    m_owner->SetPosition(position);
}
}  // namespace COA
