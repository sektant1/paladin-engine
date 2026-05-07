#include "Enemy.h"

#include <algorithm>
#include <cmath>
#include <memory>

#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/trigonometric.hpp>

#include "Engine.h"
#include "Log.h"
#include "physics/Collider.h"
#include "physics/PhysicsManager.h"
#include "physics/Raycast.h"
#include "physics/RigidBody.h"
#include "render/Material.h"
#include "render/Mesh.h"
#include "render/ParticleSystem.h"
#include "scene/components/HealthComponent.h"
#include "scene/components/MeshComponent.h"
#include "scene/components/PhysicsComponent.h"

#include <random>

namespace
{
std::shared_ptr<mnd::Texture> GetWhiteTexture()
{
    static std::shared_ptr<mnd::Texture> texture;
    if (!texture)
    {
        unsigned char white[] = {255, 255, 255, 255};
        texture = std::make_shared<mnd::Texture>(1, 1, 4, white);
    }
    return texture;
}

std::shared_ptr<mnd::Material> GetEnemyMaterial()
{
    static std::shared_ptr<mnd::Material> material;
    if (!material)
    {
        material = std::make_shared<mnd::Material>();
        material->SetShaderProgram(mnd::Engine::GetInstance().GetGraphicsAPI().GetDefaultShaderProgram());
        material->SetParam("color", mnd::vec3(1.0F, 0.0F, 0.0F));
        material->SetParam("baseColorTexture", GetWhiteTexture());
    }
    return material;
}

mnd::vec3 ReadVec3(const nlohmann::json &json, const char *key, const mnd::vec3 &fallback)
{
    if (!json.contains(key) || !json[key].is_object())
    {
        return fallback;
    }

    const auto &obj = json[key];
    return mnd::vec3(obj.value("x", fallback.x), obj.value("y", fallback.y), obj.value("z", fallback.z));
}
}  // namespace

void Enemy::LoadProperties(const nlohmann::json &json)
{
    m_visualOffset     = ReadVec3(json, "visualOffset", m_visualOffset);
    m_capsuleRadius    = std::max(0.05F, json.value("capsuleRadius", m_capsuleRadius));
    m_capsuleHeight    = std::max(m_capsuleRadius * 2.0F, json.value("capsuleHeight", m_capsuleHeight));
    m_colliderHalfExtents = ReadVec3(json, "colliderHalfExtents", m_colliderHalfExtents);
    m_maxHealth        = std::max(1, json.value("maxHealth", m_maxHealth));
    m_moveSpeed        = std::max(0.0F, json.value("moveSpeed", m_moveSpeed));
    m_detectionRange   = std::max(0.0F, json.value("detectionRange", m_detectionRange));
    m_stopDistance     = std::max(0.0F, json.value("stopDistance", m_stopDistance));
}

void Enemy::Init()
{
    EnsureHealth();
    EnsurePhysics();
    CreateCapsuleVisual();
}

void Enemy::Update(mnd::f32 deltaTime)
{
    auto *target = FindTarget();
    if (target != nullptr && m_moveSpeed > 0.0F)
    {
        mnd::vec3 toTarget = target->GetWorldPosition() - GetWorldPosition();
        toTarget.y = 0.0F;

        const mnd::f32 distance = glm::length(toTarget);
        if (distance > m_stopDistance && distance <= m_detectionRange)
        {
            const mnd::vec3 dir       = toTarget / distance;
            const mnd::vec3 stepWorld = dir * m_moveSpeed * deltaTime;
            const mnd::vec3 allowed   = ResolveAxisMove(stepWorld);
            SetPosition(GetPosition() + allowed);

            const mnd::f32 yaw = std::atan2(dir.x, dir.z);
            SetRotation(glm::angleAxis(yaw, mnd::vec3(0.0F, 1.0F, 0.0F)));
        }
    }

    if (m_rigidBody)
    {
        m_rigidBody->SetPosition(GetWorldPosition());
        m_rigidBody->SetRotation(GetWorldRotation());
    }

    mnd::GameObject::Update(deltaTime);
}

mnd::vec3 Enemy::ResolveAxisMove(const mnd::vec3 &delta) const
{
    mnd::vec3 result(0.0F);
    if (std::abs(delta.x) > 0.0F && !AxisBlocked(0, delta.x))
    {
        result.x = delta.x;
    }
    if (std::abs(delta.z) > 0.0F && !AxisBlocked(2, delta.z))
    {
        result.z = delta.z;
    }
    result.y = delta.y;
    return result;
}

bool Enemy::AxisBlocked(int axis, mnd::f32 step) const
{
    if (axis != 0 && axis != 2)
    {
        return false;
    }

    auto &physics = mnd::Engine::GetInstance().GetPhysicsManager();

    const mnd::f32 sign      = step > 0.0F ? 1.0F : -1.0F;
    const mnd::f32 distance  = std::abs(step);
    const mnd::f32 skin      = 0.05F;
    const mnd::f32 halfAxis  = (axis == 0 ? m_colliderHalfExtents.x : m_colliderHalfExtents.z);
    const mnd::f32 halfSide  = (axis == 0 ? m_colliderHalfExtents.z : m_colliderHalfExtents.x);
    const mnd::f32 halfY     = m_colliderHalfExtents.y;

    const mnd::vec3 worldPos = GetWorldPosition();

    mnd::vec3 axisDir(0.0F);
    mnd::vec3 sideDir(0.0F);
    if (axis == 0)
    {
        axisDir.x = sign;
        sideDir.z = 1.0F;
    }
    else
    {
        axisDir.z = sign;
        sideDir.x = 1.0F;
    }

    const mnd::vec3 baseOrigin = worldPos + axisDir * (halfAxis + skin);
    const mnd::vec3 endOffset  = axisDir * distance;

    // Three rays at low/middle/high body height, plus three side offsets to
    // catch corners. 3x3 sampling pattern.
    const mnd::f32 yOffsets[3]    = {-halfY * 0.6F, 0.0F, halfY * 0.6F};
    const mnd::f32 sideOffsets[3] = {-(halfSide - skin), 0.0F, (halfSide - skin)};

    for (mnd::f32 dy : yOffsets)
    {
        for (mnd::f32 ds : sideOffsets)
        {
            const mnd::vec3 origin = baseOrigin + mnd::vec3(0.0F, dy, 0.0F) + sideDir * ds;
            const mnd::vec3 endPt  = origin + endOffset;
            mnd::RayHit     hit;
            if (physics.Raycast(origin, endPt, hit))
            {
                if (hit.object != this && hit.object != m_visualRoot)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

mnd::GameObject *Enemy::FindTarget() const
{
    auto *scene = const_cast<Enemy *>(this)->GetScene();
    return scene != nullptr ? scene->GetMainCamera() : nullptr;
}

void Enemy::EnsureHealth()
{
    auto *health = GetComponent<mnd::HealthComponent>();
    if (health == nullptr)
    {
        health = new mnd::HealthComponent();
        AddComponent(health);
    }

    health->SetMax(m_maxHealth);
    health->SetCurrent(m_maxHealth);
    health->SetDestroyOnDeath(true);

    health->OnDeath([this](const mnd::DamageInfo & /*info*/) { SpawnDeathParticles(); });
}

void Enemy::SpawnDeathParticles()
{
    auto &particles = mnd::Engine::GetInstance().GetParticleSystem();

    static std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<mnd::f32> horiz(-4.5F, 4.5F);
    std::uniform_real_distribution<mnd::f32> vert(1.0F, 6.5F);
    std::uniform_real_distribution<mnd::f32> life(0.7F, 1.4F);
    std::uniform_real_distribution<mnd::f32> sizeJitter(0.30F, 0.55F);
    std::uniform_real_distribution<mnd::f32> tintJitter(0.85F, 1.0F);

    const mnd::vec3 origin = GetWorldPosition() + mnd::vec3(0.0F, m_capsuleHeight * 0.5F, 0.0F);

    constexpr int kBurstCount = 96;
    for (int i = 0; i < kBurstCount; ++i)
    {
        const mnd::f32 tint = tintJitter(rng);

        mnd::Particle p;
        p.position     = origin;
        p.velocity     = mnd::vec3(horiz(rng), vert(rng), horiz(rng));
        p.acceleration = mnd::vec3(0.0F, -7.5F, 0.0F);
        p.lifetime     = life(rng);
        p.age          = 0.0F;
        p.sizeStart    = sizeJitter(rng);
        p.sizeEnd      = 0.05F;
        // Above-1.0 channels get amplified by the additive blend → punchy bloom-style red.
        p.colorStart   = mnd::vec4(2.6F * tint, 0.35F * tint, 0.20F * tint, 1.0F);
        p.colorEnd     = mnd::vec4(0.55F, 0.02F, 0.02F, 0.0F);
        particles.Spawn(p);
    }

    // Bright core flash — large, short-lived, fully white-hot pop.
    constexpr int kCoreCount = 12;
    for (int i = 0; i < kCoreCount; ++i)
    {
        mnd::Particle p;
        p.position     = origin;
        p.velocity     = mnd::vec3(horiz(rng) * 0.3F, vert(rng) * 0.3F, horiz(rng) * 0.3F);
        p.acceleration = mnd::vec3(0.0F);
        p.lifetime     = 0.25F;
        p.age          = 0.0F;
        p.sizeStart    = 1.1F;
        p.sizeEnd      = 0.2F;
        p.colorStart   = mnd::vec4(3.5F, 1.6F, 0.9F, 1.0F);
        p.colorEnd     = mnd::vec4(1.5F, 0.1F, 0.05F, 0.0F);
        particles.Spawn(p);
    }
}

void Enemy::EnsurePhysics()
{
    if (GetComponent<mnd::PhysicsComponent>() != nullptr)
    {
        return;
    }

    auto collider = std::make_shared<mnd::BoxCollider>(m_colliderHalfExtents);
    m_rigidBody = std::make_shared<mnd::RigidBody>(mnd::BodyType::Kinematic, collider, 0.0F, 0.8F);
    AddComponent(new mnd::PhysicsComponent(m_rigidBody));
}

void Enemy::CreateCapsuleVisual()
{
    if (m_visualRoot != nullptr || GetScene() == nullptr)
    {
        return;
    }

    m_visualRoot = GetScene()->CreateObject(GetName() + "_Visual", this);
    m_visualRoot->SetName(GetName() + "_Visual");
    m_visualRoot->SetPosition(m_visualOffset);
    m_visualRoot->AddComponent(
        new mnd::MeshComponent(GetEnemyMaterial(), mnd::Mesh::CreateCapsule(m_capsuleRadius, m_capsuleHeight, 24, 8)));
}
