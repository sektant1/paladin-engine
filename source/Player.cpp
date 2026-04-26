#include <memory>

#include "Player.h"

#include <GLFW/glfw3.h>

#include "Bullet.h"
#include "Monad.h"
#include "GameConstants.h"
#include "physics/Collider.h"
#include "physics/RigidBody.h"
#include "render/Material.h"
#include "render/Mesh.h"
#include "scene/GameObject.h"
#include "scene/components/AnimationComponent.h"
#include "scene/components/AudioComponent.h"
#include "scene/components/MeshComponent.h"
#include "scene/components/PhysicsComponent.h"
#include "scene/components/PlayerControllerComponent.h"

using namespace mnd;

void Player::Init()
{
    if (auto bullet = FindChildByName(kChildBulletName))
    {
        bullet->SetActive(false);
    }

    if (auto fire = FindChildByName(kChildBoomName))
    {
        fire->SetActive(false);
    }

    if (auto gun = FindChildByName(kChildGunName))
    {
        m_animationComponent = gun->GetComponent<mnd::AnimationComponent>();
    }

    m_audioComponent            = GetComponent<mnd::AudioComponent>();
    m_playerControllerComponent = GetComponent<mnd::PlayerControllerComponent>();
}

void Player::Update(f32 deltaTime)
{
    auto &input = Engine::GetInstance().GetInputManager();
    if (input.IsMouseButtonPressed(MouseButton::Left))
    {
        if (m_animationComponent && !m_animationComponent->IsPlaying())
        {
            m_animationComponent->Play(kAnimShoot, false);

            if (m_audioComponent)
            {
                if (m_audioComponent->IsPlaying(kSfxShoot))
                {
                    m_audioComponent->Stop(kSfxShoot);
                }
                m_audioComponent->Play(kSfxShoot);
            }

            auto bullet   = GetScene()->CreateObject<Bullet>("bullet");
            auto material = mnd::Material::Load("materials/bullet.mat");
            auto mesh     = mnd::Mesh::CreateSphere(0.1f, 32, 32);

            bullet->AddComponent(new mnd::MeshComponent(material, mesh));

            vec3 pos = vec3(0.0f);
            if (auto child = FindChildByName(kChildBoomName))
            {
                pos = child->GetWorldPosition();
            }
            bullet->SetPosition(pos + GetRotation() * vec3(-0.2f, 0.2f, -1.75f));

            auto collider  = std::make_shared<mnd::SphereCollider>(0.2f);
            auto rigidBody = std::make_shared<RigidBody>(mnd::BodyType::Dynamic, collider, 5.0f, 1.0f);
            bullet->AddComponent(new mnd::PhysicsComponent(rigidBody));

            // CCD: bullet moves ~1.67 u/frame at 60fps, way past sphere radius 0.2 — needs swept test.
            rigidBody->EnableCcd(0.1f, 0.15f);

            vec3 front = GetRotation() * vec3(0.0f, 0.0f, -1.0f);
            rigidBody->ApplyImpulse(front * 500.0f);
        }
    }

    if (input.IsKeyPressed(Key::Space))
    {
        if (m_audioComponent && !m_audioComponent->IsPlaying(kSfxJump) && m_playerControllerComponent->OnGround())
        {
            m_audioComponent->Play(kSfxJump);
        }
    }

    // clang-format off
    bool walking = 
        input.IsKeyPressed(Key::W) || 
        input.IsKeyPressed(Key::A) || 
        input.IsKeyPressed(Key::S) || 
        input.IsKeyPressed(Key::D);
    // clang-format on

    if (walking && m_playerControllerComponent && m_playerControllerComponent->OnGround())
    {
        if (m_audioComponent && !m_audioComponent->IsPlaying(kSfxStep))
        {
            m_audioComponent->Play(kSfxStep, true);
        }
    } else
    {
        if (m_audioComponent && m_audioComponent->IsPlaying(kSfxStep))
        {
            m_audioComponent->Stop(kSfxStep);
        }
    }
    if (input.IsKeyPressed(Key::LeftShift) && walking)
    {
        m_playerControllerComponent->SetMoveSpeed(kPlayerRunSpeed);
    } else
    {
        m_playerControllerComponent->SetMoveSpeed(kPlayerWalkSpeed);
    }

    mnd::GameObject::Update(deltaTime);
}
