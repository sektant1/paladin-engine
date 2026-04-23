#include "scene/components/CameraComponent.h"

#include "Engine.h"
#include "GLFW/glfw3.h"
#include "Types.h"
#include "scene/GameObject.h"

namespace COA
{

void CameraComponent::Update(f32 deltaTime) {}

mat4 CameraComponent::GetViewMatrix() const
{
    mat4 mat = mat4(1.0F);
    mat      = mat4_cast(m_owner->GetRotation());
    mat[3]   = vec4(m_owner->GetPosition(), 1.0F);

    if (m_owner->GetParent() != nullptr) {
        mat = m_owner->GetParent()->GetWorldTransform() * mat;
    }

    return inverse(mat);
}

mat4 CameraComponent::GetProjectionMatrix(f32 aspect) const
{
    return perspective(radians(m_fov), aspect, m_nearPlane, m_farPlane);
}

}  // namespace COA
