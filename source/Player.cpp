#include "Player.h"

#include <memory>

#include <GLFW/glfw3.h>

#include "Bullet.h"
#include "COA.h"
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

using namespace COA;

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
        m_animationComponent = gun->GetComponent<COA::AnimationComponent>();
    }

    m_audioComponent            = GetComponent<COA::AudioComponent>();
    m_playerControllerComponent = GetComponent<COA::PlayerControllerComponent>();
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
            auto material = COA::Material::Load("materials/suzanne.mat");
            auto mesh     = COA::Mesh::CreateSphere(0.2f, 32, 32);

            bullet->AddComponent(new COA::MeshComponent(material, mesh));

            vec3 pos = vec3(0.0f);
            if (auto child = FindChildByName(kChildBoomName))
            {
                pos = child->GetWorldPosition();
            }
            bullet->SetPosition(pos + GetRotation() * vec3(-0.2f, 0.2f, -1.75f));

            auto collider  = std::make_shared<COA::SphereCollider>(0.2f);
            auto rigidBody = std::make_shared<RigidBody>(COA::BodyType::Dynamic, collider, 10.0f, 0.1f);
            bullet->AddComponent(new COA::PhysicsComponent(rigidBody));

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

    COA::GameObject::Update(deltaTime);
}
