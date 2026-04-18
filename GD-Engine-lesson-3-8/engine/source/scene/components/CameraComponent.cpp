#include "scene/components/CameraComponent.h"
#include "scene/GameObject.h"
#include <glm/gtc/matrix_transform.hpp>

namespace eng
{
    void CameraComponent::Update(float deltaTime)
    {
    }

    glm::mat4 CameraComponent::GetViewMatrix() const
    {
        return glm::inverse(m_owner->GetWorldTransform());
    }

    glm::mat4 CameraComponent::GetProjectionMatrix(float aspect) const
    {
        return glm::perspective(glm::radians(m_fov), aspect, m_nearPlane, m_farPlane);
    }
}