#include "scene/components/SpriteComponent.h"
#include "graphics/Texture.h"
#include "render/RenderQueue.h"
#include "scene/GameObject.h"
#include "Engine.h"

#include <string>

namespace eng
{
    void SpriteComponent::LoadProperties(const nlohmann::json& json)
    {
        // Texture
        const std::string texturePath = json.value("texture", "");
        if (auto texture = Texture::Load(texturePath))
        {
            SetTexture(texture);
        }

        // Color
        if (json.contains("color"))
        {
            auto& colorObj = json["color"];
            glm::vec4 color(
                colorObj.value("r", 1.0f),
                colorObj.value("g", 1.0f),
                colorObj.value("b", 1.0f),
                colorObj.value("a", 1.0f)
            );
            SetColor(color);
        }

        // Size
        if (json.contains("size"))
        {
            auto& sizeObj = json["size"];
            glm::vec2 size(
                sizeObj.value("x", 100.0f),
                sizeObj.value("y", 100.0f)
            );
            SetSize(size);
        }

        // Lower Left UV
        if (json.contains("lowerLeftUV"))
        {
            auto& uvObj = json["lowerLeftUV"];
            glm::vec2 uv(
                uvObj.value("u", 0.0f),
                uvObj.value("v", 0.0f)
            );
            SetLowerLeftUV(uv);
        }

        // Upper Right UV
        if (json.contains("upperRightUV"))
        {
            auto& uvObj = json["upperRightUV"];
            glm::vec2 uv(
                uvObj.value("u", 1.0f),
                uvObj.value("v", 1.0f)
            );
            SetUpperRightUV(uv);
        }

        // Pivot
        if (json.contains("pivot"))
        {
            auto& pivotObj = json["pivot"];
            glm::vec2 pivot(
                pivotObj.value("x", 0.5f),
                pivotObj.value("y", 0.5f)
            );
            SetPivot(pivot);
        }
    }

    void SpriteComponent::Update(float deltaTime)
    {
        if (!m_texture || !m_visible)
        {
            return;
        }

        RenderCommand2D command;
        command.modelMatrix = GetOwner()->GetWorldTransform2D();
        command.texture = m_texture.get();
        command.color = m_color;
        command.size = m_size;
        command.lowerLeftUV = m_lowerLeftUV;
        command.upperRightUV = m_upperRightUV;
        command.pivot = m_pivot;

        auto& renderQueue = Engine::GetInstance().GetRenderQueue();
        renderQueue.Submit(command);
    }

    void SpriteComponent::SetTexture(const std::shared_ptr<Texture>& texture)
    {
        m_texture = texture;
    }

    const std::shared_ptr<Texture>& SpriteComponent::GetTexture() const
    {
        return m_texture;
    }

    void SpriteComponent::SetColor(const glm::vec4& color)
    {
        m_color = color;
    }

    const glm::vec4& SpriteComponent::GetColor() const
    {
        return m_color;
    }

    void SpriteComponent::SetSize(const glm::vec2& size)
    {
        m_size = size;
    }

    const glm::vec2& SpriteComponent::GetSize() const
    {
        return m_size;
    }

    void SpriteComponent::SetLowerLeftUV(const glm::vec2& uv)
    {
        m_lowerLeftUV = uv;
    }

    const glm::vec2& SpriteComponent::GetLowerLeftUV() const
    {
        return m_lowerLeftUV;
    }

    void SpriteComponent::SetUpperRightUV(const glm::vec2& uv)
    {
        m_upperRightUV = uv;
    }

    const glm::vec2& SpriteComponent::GetUpperRightUV() const
    {
        return m_upperRightUV;
    }

    void SpriteComponent::SetUV(const glm::vec2& lowerLeftUV, const glm::vec2& upperRightUV)
    {
        m_lowerLeftUV = lowerLeftUV;
        m_upperRightUV = upperRightUV;
    }

    void SpriteComponent::SetPivot(const glm::vec2& pivot)
    {
        m_pivot = pivot;
    }

    const glm::vec2& SpriteComponent::GetPivot() const
    {
        return m_pivot;
    }

    void SpriteComponent::SetVisibile(bool visible)
    {
        m_visible = visible;
    }

    bool SpriteComponent::IsVisible() const
    {
        return m_visible;
    }
}