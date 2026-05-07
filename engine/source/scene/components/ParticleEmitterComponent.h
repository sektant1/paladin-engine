/**
 * @file ParticleEmitterComponent.h
 * @ingroup mnd_components
 * @brief Spawns particles each frame at the owner's world position.
 *
 * Pushes Particle instances into Engine::GetParticleSystem(). All emission
 * parameters are JSON-tunable. Random ranges are uniform between *Min and *Max.
 */

#pragma once

#include <random>

#include "scene/Component.h"

namespace mnd
{

class ParticleEmitterComponent : public Component
{
    COMPONENT(ParticleEmitterComponent)
 public:
    void Update(f32 deltaTime) override;
    void LoadProperties(const nlohmann::json &json) override;

    void SetEmitting(bool on) { m_emitting = on; }
    [[nodiscard]] bool IsEmitting() const { return m_emitting; }

    /// Single-shot burst at the owner's current position (e.g. on death/hit).
    void Burst(int count);

 private:
    void EmitOne();

    bool m_emitting = true;
    f32  m_rate     = 30.0F;   ///< Particles per second.
    f32  m_accum    = 0.0F;    ///< Internal time accumulator.

    vec3 m_offset{0.0F};       ///< World offset added to owner position.
    vec3 m_velocityMin{-0.5F, 1.0F, -0.5F};
    vec3 m_velocityMax{0.5F, 2.5F, 0.5F};
    vec3 m_acceleration{0.0F, -2.0F, 0.0F};

    f32  m_lifetimeMin = 0.6F;
    f32  m_lifetimeMax = 1.2F;
    f32  m_sizeStart   = 0.18F;
    f32  m_sizeEnd     = 0.0F;

    vec4 m_colorStart{1.0F, 0.85F, 0.45F, 1.0F};
    vec4 m_colorEnd{1.0F, 0.20F, 0.10F, 0.0F};

    std::mt19937 m_rng{std::random_device{}()};
};

}  // namespace mnd
