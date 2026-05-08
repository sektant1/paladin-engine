/**
 * @file ParticleSystem.h
 * @ingroup mnd_render
 * @brief Pool-allocated CPU particles drawn as additive billboards.
 *
 * The Engine owns one ParticleSystem. Emitter components push particles into
 * it via Spawn(); each frame Update() ages them, then Render() issues one
 * billboarded draw per live particle right after the lit pass.
 *
 * Scope: hundreds of particles, no instancing yet. Cheap enough for the
 * boomer-shooter FX budget (sparks, blood, muzzle flash, torches). If counts
 * grow into the thousands, the obvious upgrade is a single instanced draw
 * with a streaming VBO of per-particle data.
 */

#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "Types.h"

namespace mnd
{
class Mesh;
class ShaderProgram;

struct CameraData;

struct Particle
{
    vec3  position {0.0F};
    vec3  velocity {0.0F};
    vec3  acceleration {0.0F};  ///< World-space gravity / drag, applied each frame.
    vec4  colorStart {1.0F};
    vec4  colorEnd {1.0F};
    float age       = 0.0F;
    float lifetime  = 1.0F;
    float sizeStart = 0.2F;
    float sizeEnd   = 0.0F;

    [[nodiscard]] bool IsAlive() const { return age < lifetime; }
};

class ParticleSystem
{
 public:
    void Init();
    void Shutdown();

    /// Step every live particle, swap-erase dead ones.
    void Update(f32 deltaTime);

    /// Camera-aligned billboard pass. Call after the lit pass, before UI.
    void Render(const CameraData &cameraData);

    /// Push a fully-configured particle into the pool. Drops oldest if full.
    void Spawn(const Particle &particle);

    void Clear() { m_particles.clear(); }

    [[nodiscard]] std::size_t GetAliveCount() const { return m_particles.size(); }

    [[nodiscard]] std::size_t GetCapacity() const { return m_capacity; }

    void SetPaused(bool paused) { m_paused = paused; }

    [[nodiscard]] bool IsPaused() const { return m_paused; }

 private:
    std::shared_ptr<Mesh>          m_quad;
    std::shared_ptr<ShaderProgram> m_shader;
    std::vector<Particle>          m_particles;
    std::size_t                    m_capacity = 4096;
    bool                           m_paused   = false;
};

}  // namespace mnd
