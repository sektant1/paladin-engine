#pragma once

#include "scene/Component.h"

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>
#include <memory>

namespace eng
{
    class Texture;

    class SpriteComponent : public Component
    {
        COMPONENT(SpriteComponent)
    public:
        void LoadProperties(const nlohmann::json& json) override;
        void Update(float deltaTime) override;

        void SetTexture(const std::shared_ptr<Texture>& texture);
        const std::shared_ptr<Texture>& GetTexture() const;
        void SetColor(const glm::vec4& color);
        const glm::vec4& GetColor() const;
        void SetSize(const glm::vec2& size);
        const glm::vec2& GetSize() const;
        void SetLowerLeftUV(const glm::vec2& uv);
        const glm::vec2& GetLowerLeftUV() const;
        void SetUpperRightUV(const glm::vec2& uv);
        const glm::vec2& GetUpperRightUV() const;
        void SetUV(const glm::vec2& lowerLeftUV, const glm::vec2& upperRightUV);
        void SetPivot(const glm::vec2& pivot);
        const glm::vec2& GetPivot() const;
        void SetVisibile(bool visible);
        bool IsVisible() const;

    private:
        std::shared_ptr<Texture> m_texture;
        glm::vec4 m_color = glm::vec4(1.0f);
        glm::vec2 m_size = glm::vec2(100.0f);
        glm::vec2 m_lowerLeftUV = glm::vec2(0.0f);
        glm::vec2 m_upperRightUV = glm::vec2(1.0f);
        glm::vec2 m_pivot = glm::vec2(0.5f);
        bool m_visible = true;
    };
}