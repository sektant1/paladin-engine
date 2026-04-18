#include "scene/components/CameraComponent.h"

#include "Types.h"
#include "scene/GameObject.h"

namespace ENG
{

void CameraComponent::Update(f32 deltaTime) {}

mat4 CameraComponent::GetViewMatrix() const
{
    return inverse(m_owner->GetWorldTransform());
}

mat4 CameraComponent::GetProjectionMatrix(f32 aspect) const
{
    return perspective(radians(m_fov), aspect, m_nearPlane, m_farPlane);
}

}  // namespace ENG
