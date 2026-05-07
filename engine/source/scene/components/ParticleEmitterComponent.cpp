#include "scene/components/ParticleEmitterComponent.h"

#include <algorithm>

#include "Engine.h"
#include "render/ParticleSystem.h"
#include "scene/GameObject.h"

namespace mnd
{

namespace
{
vec3 ReadVec3(const nlohmann::json &json, const char *key, const vec3 &fallback)
{
    if (!json.contains(key) || !json[key].is_object())
    {
        return fallback;
    }
    const auto &obj = json[key];
    return vec3(obj.value("x", fallback.x), obj.value("y", fallback.y), obj.value("z", fallback.z));
}

vec4 ReadVec4(const nlohmann::json &json, const char *key, const vec4 &fallback)
{
    if (!json.contains(key) || !json[key].is_object())
    {
        return fallback;
    }
    const auto &obj = json[key];
    return vec4(obj.value("r", fallback.r),
                obj.value("g", fallback.g),
                obj.value("b", fallback.b),
                obj.value("a", fallback.a));
}

f32 RandRange(std::mt19937 &rng, f32 lo, f32 hi)
{
    if (hi <= lo)
    {
        return lo;
    }
    std::uniform_real_distribution<f32> dist(lo, hi);
    return dist(rng);
}
}  // namespace

void ParticleEmitterComponent::Update(f32 deltaTime)
{
    if (!m_emitting || deltaTime <= 0.0F || m_rate <= 0.0F)
    {
        return;
    }

    m_accum += deltaTime * m_rate;
    while (m_accum >= 1.0F)
    {
        EmitOne();
        m_accum -= 1.0F;
    }
}

void ParticleEmitterComponent::Burst(int count)
{
    for (int i = 0; i < count; ++i)
    {
        EmitOne();
    }
}

void ParticleEmitterComponent::EmitOne()
{
    if (m_owner == nullptr)
    {
        return;
    }

    Particle particle;
    particle.position = m_owner->GetWorldPosition() + m_offset;
    particle.velocity = vec3(RandRange(m_rng, m_velocityMin.x, m_velocityMax.x),
                             RandRange(m_rng, m_velocityMin.y, m_velocityMax.y),
                             RandRange(m_rng, m_velocityMin.z, m_velocityMax.z));
    particle.acceleration = m_acceleration;
    particle.lifetime     = std::max(0.05F, RandRange(m_rng, m_lifetimeMin, m_lifetimeMax));
    particle.age          = 0.0F;
    particle.sizeStart    = m_sizeStart;
    particle.sizeEnd      = m_sizeEnd;
    particle.colorStart   = m_colorStart;
    particle.colorEnd     = m_colorEnd;

    Engine::GetInstance().GetParticleSystem().Spawn(particle);
}

void ParticleEmitterComponent::LoadProperties(const nlohmann::json &json)
{
    m_emitting     = json.value("emitting", m_emitting);
    m_rate         = std::max(0.0F, json.value("rate", m_rate));
    m_offset       = ReadVec3(json, "offset", m_offset);
    m_velocityMin  = ReadVec3(json, "velocityMin", m_velocityMin);
    m_velocityMax  = ReadVec3(json, "velocityMax", m_velocityMax);
    m_acceleration = ReadVec3(json, "acceleration", m_acceleration);
    m_lifetimeMin  = std::max(0.05F, json.value("lifetimeMin", m_lifetimeMin));
    m_lifetimeMax  = std::max(m_lifetimeMin, json.value("lifetimeMax", m_lifetimeMax));
    m_sizeStart    = std::max(0.0F, json.value("sizeStart", m_sizeStart));
    m_sizeEnd      = std::max(0.0F, json.value("sizeEnd", m_sizeEnd));
    m_colorStart   = ReadVec4(json, "colorStart", m_colorStart);
    m_colorEnd     = ReadVec4(json, "colorEnd", m_colorEnd);
}

}  // namespace mnd
