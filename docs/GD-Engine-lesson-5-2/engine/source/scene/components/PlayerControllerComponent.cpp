#include "scene/components/PlayerControllerComponent.h"
#include "input/InputManager.h"
#include "Engine.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec4.hpp>

namespace eng
{
    void PlayerControllerComponent::Init()
    {
        m_kinematicController = std::make_unique<KinematicCharacterController>(0.4f, 1.2f, m_owner->GetWorldPosition());
    }

    void PlayerControllerComponent::Update(float deltaTime)
    {
        auto& inputManager = Engine::GetInstance().GetInputManager();
        auto rotation = m_owner->GetRotation();

        if (inputManager.IsMousePositionChanged())
        {
            const auto& oldPos = inputManager.GetMousePositionOld();
            const auto& currentPos = inputManager.GetMousePositionCurrent();

            float deltaX = currentPos.x - oldPos.x;
            float deltaY = currentPos.y - oldPos.y;

            // rot around Y axis
            float yDeltaAngle = -deltaX * m_sensitivity * deltaTime;
            m_yRot += yDeltaAngle;
            glm::quat yRot = glm::angleAxis(glm::radians(m_yRot), glm::vec3(0.0f, 1.0f, 0.0f));

            // rot around X axis
            float xDeltaAngle = -deltaY * m_sensitivity * deltaTime;
            m_xRot += xDeltaAngle;
            m_xRot = std::clamp(m_xRot, -89.0f, 89.0f);
            glm::quat xRot = glm::angleAxis(glm::radians(m_xRot), glm::vec3(1.0f, 0.0f, 0.0f));

            rotation = glm::normalize(yRot * xRot);

            m_owner->SetRotation(rotation);
        }

        glm::vec3 front = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 right = rotation * glm::vec3(1.0f, 0.0f, 0.0f);

        glm::vec3 move(0.0f);
        // Left/Right movement
        if (inputManager.IsKeyPressed(GLFW_KEY_A))
        {
            move -= right;
        }
        else if (inputManager.IsKeyPressed(GLFW_KEY_D))
        {
            move += right;
        }
        // Vertical movement
        if (inputManager.IsKeyPressed(GLFW_KEY_W))
        {
            move += front;
        }
        else if (inputManager.IsKeyPressed(GLFW_KEY_S))
        {
            move -= front;
        }

        if (inputManager.IsKeyPressed(GLFW_KEY_SPACE))
        {
            m_kinematicController->Jump(glm::vec3(0.0f, 5.0f, 0.0f));
        }

        if (glm::dot(move, move) > 0)
        {
            move = glm::normalize(move);
        }
        m_kinematicController->Walk(move * m_moveSpeed * deltaTime);

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
}