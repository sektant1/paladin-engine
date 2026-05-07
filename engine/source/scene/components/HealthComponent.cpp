#include "scene/components/HealthComponent.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

#include <GLFW/glfw3.h>

#include "Engine.h"
#include "render/SpriteRenderer.h"
#include "scene/GameObject.h"
#include "scene/Scene.h"
#include "scene/components/CameraComponent.h"

namespace mnd
{

namespace
{
vec4 LerpColor(const vec4 &a, const vec4 &b, f32 t)
{
    t = std::clamp(t, 0.0F, 1.0F);
    return a * (1.0F - t) + b * t;
}
}  // namespace

void HealthComponent::Update(f32 deltaTime)
{
    const f32 target = m_max > 0 ? static_cast<f32>(m_current) / static_cast<f32>(m_max) : 0.0F;
    const f32 t      = 1.0F - std::exp(-m_barLerpSpeed * std::max(deltaTime, 0.0F));
    m_displayedFill  = std::clamp(m_displayedFill + (target - m_displayedFill) * t, 0.0F, 1.0F);

    DrawHealthBar();
}

void HealthComponent::DrawHealthBar()
{
    if (!m_showBar || m_owner == nullptr)
    {
        return;
    }
    if (m_hideAtFull && m_current >= m_max && m_displayedFill >= 0.999F)
    {
        return;
    }

    auto &engine = Engine::GetInstance();
    auto *window = engine.GetWindow();
    auto *scene  = engine.GetScene();
    if (window == nullptr || scene == nullptr)
    {
        return;
    }

    auto *cameraObject = scene->GetMainCamera();
    if (cameraObject == nullptr)
    {
        return;
    }
    auto *camera = cameraObject->GetComponent<CameraComponent>();
    if (camera == nullptr)
    {
        return;
    }

    int winW = 0;
    int winH = 0;
    glfwGetFramebufferSize(window, &winW, &winH);
    if (winW <= 0 || winH <= 0)
    {
        return;
    }

    const f32  aspect = static_cast<f32>(winW) / static_cast<f32>(winH);
    const mat4 view   = camera->GetViewMatrix();
    const mat4 proj   = camera->GetProjectionMatrix(aspect);

    const vec3 worldPos = m_owner->GetWorldPosition() + vec3(0.0F, m_barOffsetY, 0.0F);
    const vec4 clip     = proj * view * vec4(worldPos, 1.0F);
    if (clip.w <= 0.0F)
    {
        return;
    }

    const vec3 ndc = vec3(clip) / clip.w;
    if (ndc.z < -1.0F || ndc.z > 1.0F)
    {
        return;
    }

    const f32 screenX = (ndc.x * 0.5F + 0.5F) * static_cast<f32>(winW);
    const f32 screenY = (1.0F - (ndc.y * 0.5F + 0.5F)) * static_cast<f32>(winH);

    // Perspective-correct screen width: project a world-space delta vector onto the screen.
    // For a perspective projection, a horizontal world-space length L at clip-space depth w
    // maps to L * proj[0][0] * (winW * 0.5) / w pixels.
    const f32 fovScale     = proj[0][0];
    const f32 rawPixelWide = m_barWorldWidth * fovScale * (static_cast<f32>(winW) * 0.5F) / clip.w;
    const f32 width        = std::clamp(rawPixelWide, m_minScreenWidth, m_maxScreenWidth);
    const f32 height       = std::max(8.0F, width * m_barAspect);
    const f32 border       = std::max(1.0F, std::round(height * 0.18F));
    const f32 shadowOffset = std::max(2.0F, std::round(height * 0.32F));

    const f32 halfW = width * 0.5F;
    const f32 halfH = height * 0.5F;

    auto &sprites = engine.GetSpriteRenderer();

    // Drop shadow.
    sprites.DrawRect(vec2(screenX - halfW - border + shadowOffset * 0.5F,
                          screenY - halfH - border + shadowOffset),
                     vec2(width + border * 2.0F, height + border * 2.0F),
                     m_shadowColor);

    // Outer border.
    sprites.DrawRect(vec2(screenX - halfW - border, screenY - halfH - border),
                     vec2(width + border * 2.0F, height + border * 2.0F),
                     m_borderColor);

    // Background — split into top/bottom halves for a subtle vertical gradient.
    const vec2 bgPos(screenX - halfW, screenY - halfH);
    sprites.DrawRect(bgPos, vec2(width, height * 0.5F), m_bgColorTop);
    sprites.DrawRect(vec2(bgPos.x, bgPos.y + height * 0.5F), vec2(width, height * 0.5F), m_bgColorBottom);

    // Fill.
    const f32 fill = std::clamp(m_displayedFill, 0.0F, 1.0F);
    if (fill > 0.0F)
    {
        const vec4 fillColor = (fill > 0.5F)
                                   ? LerpColor(m_fillColorMid, m_fillColorHigh, (fill - 0.5F) * 2.0F)
                                   : LerpColor(m_fillColorLow, m_fillColorMid, fill * 2.0F);

        const f32  fillW       = width * fill;
        const vec4 fillBottom  = fillColor * vec4(0.75F, 0.75F, 0.75F, 1.0F);
        sprites.DrawRect(bgPos, vec2(fillW, height * 0.5F), fillColor);
        sprites.DrawRect(vec2(bgPos.x, bgPos.y + height * 0.5F),
                         vec2(fillW, height * 0.5F),
                         vec4(fillBottom.x, fillBottom.y, fillBottom.z, fillColor.w));

        // Gloss highlight on the upper third of the fill.
        sprites.DrawRect(vec2(bgPos.x, bgPos.y + height * 0.10F),
                         vec2(fillW, height * 0.28F),
                         m_glossColor);
    }

    // Numeric label centered on the bar.
    if (m_showText)
    {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%d / %d", m_current, m_max);
        const f32 textHeight = std::clamp(height * 0.85F, 10.0F, 28.0F);
        const f32 approxCharW = textHeight * 0.5F;
        const f32 textWidth   = approxCharW * static_cast<f32>(std::strlen(buf));
        const f32 textX       = screenX - textWidth * 0.5F;
        const f32 textY       = screenY - textHeight * 0.5F;
        sprites.DrawText(buf, vec2(textX + 1.0F, textY + 1.0F), textHeight, m_textShadowColor);
        sprites.DrawText(buf, vec2(textX, textY), textHeight, m_textColor);
    }
}

void HealthComponent::LoadProperties(const nlohmann::json &json)
{
    if (json.contains("max"))
    {
        m_max = json["max"].get<int>();
    }
    m_current = json.value("current", m_max);
    m_current = std::clamp(m_current, 0, m_max);

    m_invulnerable   = json.value("invulnerable", false);
    m_destroyOnDeath = json.value("destroyOnDeath", false);

    m_showBar         = json.value("showBar", m_showBar);
    m_hideAtFull      = json.value("hideAtFull", m_hideAtFull);
    m_showText        = json.value("showText", m_showText);
    m_barOffsetY      = json.value("barOffsetY", m_barOffsetY);
    m_barWorldWidth   = json.value("barWorldWidth", m_barWorldWidth);
    m_barAspect       = json.value("barAspect", m_barAspect);
    m_minScreenWidth  = json.value("minScreenWidth", m_minScreenWidth);
    m_maxScreenWidth  = json.value("maxScreenWidth", m_maxScreenWidth);
    m_barLerpSpeed    = json.value("barLerpSpeed", m_barLerpSpeed);

    m_displayedFill = m_max > 0 ? static_cast<f32>(m_current) / static_cast<f32>(m_max) : 0.0F;
}

void HealthComponent::SetMax(int hp)
{
    m_max     = std::max(1, hp);
    m_current = std::min(m_current, m_max);
}

void HealthComponent::SetCurrent(int hp)
{
    m_current = std::clamp(hp, 0, m_max);
}

void HealthComponent::Heal(int amount)
{
    if (IsDead() || amount <= 0)
    {
        return;
    }
    m_current = std::min(m_max, m_current + amount);
}

void HealthComponent::Revive()
{
    m_current = m_max;
}

void HealthComponent::ApplyDamage(const DamageInfo &info)
{
    if (m_invulnerable || IsDead() || info.amount <= 0.0F)
    {
        return;
    }

    const int dmg = std::max(1, static_cast<int>(info.amount));
    m_current     = std::max(0, m_current - dmg);

    if (m_onDamage)
    {
        m_onDamage(info);
    }

    if (m_current == 0)
    {
        if (m_onDeath)
        {
            m_onDeath(info);
        }
        if (m_destroyOnDeath && m_owner != nullptr)
        {
            m_owner->MarkForDestroy();
        }
    }
}

}  // namespace mnd
