#pragma once

#include "scene/Component.h"

#include <glm/vec3.hpp>

namespace eng
{
    class LightComponent : public Component
    {
        COMPONENT(LightComponent)
    public:
        void LoadProperties(const nlohmann::json& json) override;
        void Update(float deltaTime) override;

        void SetColor(const glm::vec3& color);
        const glm::vec3& GetColor() const;

    private:
        glm::vec3 m_color = glm::vec3(1.0f);
    };
}