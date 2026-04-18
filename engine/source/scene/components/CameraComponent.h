#pragma once

#include "Types.h"
#include "scene/Component.h"

namespace ENG
{
class CameraComponent : public Component
{
    COMPONENT(CameraComponent)
public:
    void Update(f32 deltaTime) override;
    mat4 GetViewMatrix() const;
    mat4 GetProjectionMatrix(f32 aspect) const;

private:
    f32 m_fov       = 60.0F;
    f32 m_nearPlane = 0.1F;
    f32 m_farPlane  = 1000.0F;
};
}  // namespace ENG
