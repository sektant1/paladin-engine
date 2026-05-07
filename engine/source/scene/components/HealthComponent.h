/**
 * @file HealthComponent.h
 * @ingroup mnd_components
 * @brief Universal HP sink: damageable + killable game objects.
 *
 * Any GameObject that can take damage (player, goblins, breakables) carries
 * a HealthComponent. The DamageSystem looks for this component on the victim
 * GameObject when applying a DamageEvent. Death emits an OnDeath callback so
 * gameplay code (loot drops, ragdoll, score) can react without coupling to
 * the damage source.
 */

#pragma once

#include <functional>

#include "scene/Component.h"

namespace mnd
{

struct DamageInfo
{
    GameObject *attacker = nullptr;
    float       amount   = 0.0F;
    vec3        hitPoint{0.0F};
    vec3        hitNormal{0.0F};
};

class HealthComponent : public Component
{
    COMPONENT(HealthComponent)
 public:
    using DamageFn = std::function<void(const DamageInfo &)>;
    using DeathFn  = std::function<void(const DamageInfo &)>;

    void Update(f32 deltaTime) override;
    void LoadProperties(const nlohmann::json &json) override;

    /// Apply damage. Routed through DamageSystem::Apply, do not call directly.
    void ApplyDamage(const DamageInfo &info);

    [[nodiscard]] int  GetMax() const { return m_max; }
    [[nodiscard]] int  GetCurrent() const { return m_current; }
    [[nodiscard]] bool IsDead() const { return m_current <= 0; }
    [[nodiscard]] bool IsInvulnerable() const { return m_invulnerable; }

    void SetMax(int hp);
    void SetCurrent(int hp);
    void SetInvulnerable(bool inv) { m_invulnerable = inv; }
    void SetDestroyOnDeath(bool b) { m_destroyOnDeath = b; }

    /// Heal up to max. No-op if dead (use Revive for that).
    void Heal(int amount);

    /// Reset to full HP and clear dead state.
    void Revive();

    /// Subscribe to damage events. Listeners fire on every non-zero damage tick.
    void OnDamage(DamageFn fn) { m_onDamage = std::move(fn); }
    /// Subscribe to death event. Fires once per transition from alive→dead.
    void OnDeath(DeathFn fn) { m_onDeath = std::move(fn); }

    /// Toggle on-screen 2D bar.
    void SetShowBar(bool show) { m_showBar = show; }
    [[nodiscard]] bool ShowsBar() const { return m_showBar; }

 private:
    void DrawHealthBar();

    int      m_max            = 100;
    int      m_current        = 100;
    bool     m_invulnerable   = false;
    bool     m_destroyOnDeath = false;
    DamageFn m_onDamage;
    DeathFn  m_onDeath;

    // 2D health bar overlay.
    bool m_showBar         = true;
    bool m_hideAtFull      = false;
    bool m_showText        = true;
    f32  m_barOffsetY      = 1.1F;    ///< World-space offset above owner origin.
    f32  m_barWorldWidth   = 1.6F;    ///< World-space reference width — scales with camera distance.
    f32  m_barAspect       = 0.18F;   ///< Height = width * aspect.
    f32  m_minScreenWidth  = 32.0F;   ///< Pixel floor so distant bars stay readable.
    f32  m_maxScreenWidth  = 360.0F;  ///< Pixel ceiling so close-up bars don't dominate.
    f32  m_displayedFill   = 1.0F;    ///< Smoothed [0,1] fill ratio.
    f32  m_barLerpSpeed    = 8.0F;    ///< Per-second lerp toward target ratio.
    vec4 m_shadowColor     = vec4(0.0F, 0.0F, 0.0F, 0.55F);
    vec4 m_borderColor     = vec4(0.04F, 0.05F, 0.07F, 0.95F);
    vec4 m_bgColorTop      = vec4(0.10F, 0.11F, 0.13F, 0.92F);
    vec4 m_bgColorBottom   = vec4(0.18F, 0.20F, 0.23F, 0.92F);
    vec4 m_fillColorHigh   = vec4(0.30F, 0.92F, 0.45F, 1.0F);
    vec4 m_fillColorMid    = vec4(0.98F, 0.82F, 0.25F, 1.0F);
    vec4 m_fillColorLow    = vec4(0.95F, 0.25F, 0.25F, 1.0F);
    vec4 m_glossColor      = vec4(1.0F, 1.0F, 1.0F, 0.22F);
    vec4 m_textColor       = vec4(1.0F, 1.0F, 1.0F, 0.95F);
    vec4 m_textShadowColor = vec4(0.0F, 0.0F, 0.0F, 0.85F);
};

}  // namespace mnd
