#include <memory>

#include "Player.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/trigonometric.hpp>
#include <imgui.h>

#include "Bullet.h"
#include "GameConstants.h"
#include "Monad.h"
#include "animation/IdleClip.h"
#include "animation/SkeletalAnimationClip.h"
#include "animation/Skeleton.h"
#include "editor/Editor.h"
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

    m_gunObject = FindChildByName(kChildGunName);
    if (m_gunObject)
    {
        m_animationComponent = m_gunObject->GetComponent<mnd::AnimationComponent>();
    }

    m_handsObject = FindChildByName(kChildHandsName);
    if (m_handsObject)
    {
        m_handsAnimComponent = m_handsObject->GetComponent<mnd::AnimationComponent>();
        if (m_handsAnimComponent)
        {
            // ModelImporter pre-registered a procedural "idle" clip when the FBX
            // shipped rigged-but-unanimated. Just kick it off so the hands breathe.
            // m_handsAnimComponent->Play(kAnimIdle, true);
        }
    }

    SetViewmodel(/*useHands=*/false);

    // Seed tweaker state from whatever the scene JSON specified, so the
    // panel's first frame mirrors the asset values.
    if (m_handsObject)
    {
        m_handsPosition = m_handsObject->GetPosition();
        m_handsScale    = m_handsObject->GetScale().x;  // assumed uniform from scene JSON
        glm::vec3 eul   = glm::eulerAngles(m_handsObject->GetRotation());
        m_handsRotEuler = glm::degrees(eul);
    }

    m_audioComponent            = GetComponent<mnd::AudioComponent>();
    m_playerControllerComponent = GetComponent<mnd::PlayerControllerComponent>();

    RegisterHandsTweakerPanel();
}

void Player::RebuildIdleClip()
{
    if (!m_handsAnimComponent)
    {
        return;
    }
    auto skel = m_handsAnimComponent->GetSkeleton();
    if (!skel)
    {
        return;
    }
    auto clip = mnd::MakeBreathingIdleClip(*skel, m_idleDuration, m_idleAmpScale);
    m_handsAnimComponent->RegisterSkeletalClip(clip->name, clip);
    m_handsAnimComponent->Play(clip->name, true);
}

void Player::RegisterHandsTweakerPanel()
{
    auto &editor = mnd::Engine::GetInstance().GetEditor();
    editor.RegisterPanel(
        "Hands Tweaker",
        [this]()
        {
            ImGui::TextUnformatted("Hands view-model");
            ImGui::Separator();

            // Active view-model toggle.
            int mode = m_usingHands ? 0 : 1;
            ImGui::RadioButton("Hands (1)", &mode, 0);
            ImGui::SameLine();
            ImGui::RadioButton("Gun (2)", &mode, 1);
            SetViewmodel(mode == 0);

            ImGui::Separator();
            ImGui::TextUnformatted("Transform");

            bool dirtyXform = false;
            dirtyXform |= ImGui::DragFloat3("Position", &m_handsPosition.x, 0.005f, -3.0f, 3.0f, "%.3f");
            dirtyXform |= ImGui::DragFloat3("Rotation (deg)", &m_handsRotEuler.x, 0.5f, -180.0f, 180.0f, "%.2f");
            dirtyXform |= ImGui::DragFloat("Uniform scale", &m_handsScale, 0.0005f, 0.0001f, 5.0f, "%.5f");

            if (dirtyXform && m_handsObject)
            {
                m_handsObject->SetPosition(m_handsPosition);
                glm::vec3 rad = glm::radians(m_handsRotEuler);
                m_handsObject->SetRotation(glm::quat(rad));
                m_handsObject->SetScale(mnd::vec3(m_handsScale));
            }

            if (ImGui::Button("Reset xform"))
            {
                m_handsPosition = mnd::vec3(0.0f, -0.4f, -0.6f);
                m_handsRotEuler = mnd::vec3(0.0f);
                m_handsScale    = 0.01f;
                if (m_handsObject)
                {
                    m_handsObject->SetPosition(m_handsPosition);
                    m_handsObject->SetRotation(glm::quat(glm::radians(m_handsRotEuler)));
                    m_handsObject->SetScale(mnd::vec3(m_handsScale));
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Print JSON"))
            {
                LOG_INFO(
                "[Hands] paste into scene JSON:\n"
                "  \"position\": { \"x\": %.4f, \"y\": %.4f, \"z\": %.4f },\n"
                "  \"rotation_deg\": { \"x\": %.2f, \"y\": %.2f, \"z\": %.2f },\n"
                "  \"scale\": { \"x\": %.5f, \"y\": %.5f, \"z\": %.5f }",
                m_handsPosition.x, m_handsPosition.y, m_handsPosition.z,
                m_handsRotEuler.x, m_handsRotEuler.y, m_handsRotEuler.z,
                m_handsScale, m_handsScale, m_handsScale);
            }

            ImGui::Separator();
            ImGui::TextUnformatted("Idle clip");

            ImGui::SliderFloat("Duration (s)", &m_idleDuration, 0.5f, 12.0f, "%.2f");
            ImGui::SliderFloat("Amplitude x", &m_idleAmpScale, 0.0f, 4.0f, "%.2f");

            if (ImGui::Button("Regenerate idle"))
            {
                RebuildIdleClip();
            }
            ImGui::SameLine();
            if (ImGui::Button("Play idle"))
            {
                if (m_handsAnimComponent)
                {
                    m_handsAnimComponent->Play(kAnimIdle, true);
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Stop"))
            {
                if (m_handsAnimComponent)
                {
                    m_handsAnimComponent->Stop();
                }
            }

            ImGui::Separator();
            if (m_handsAnimComponent)
            {
                ImGui::Text(
                    "Skeleton bones: %zu",
                    m_handsAnimComponent->GetSkeleton() ? m_handsAnimComponent->GetSkeleton()->Size() : 0
                );
                ImGui::Text("Playing: %s", m_handsAnimComponent->IsPlaying() ? "yes" : "no");
            } else
            {
                ImGui::TextDisabled("No Hands AnimationComponent (Hands child missing in scene?)");
            }
        }
    );
}

void Player::SetViewmodel(bool useHands)
{
    m_usingHands = useHands;
    if (m_gunObject)
    {
        m_gunObject->SetActive(!useHands);
    }
    if (m_handsObject)
    {
        m_handsObject->SetActive(useHands);
    }
}

void Player::Update(f32 deltaTime)
{
    auto &input = Engine::GetInstance().GetInputManager();

    if (input.IsKeyPressed(Key::Num1))
    {
        SetViewmodel(true);
    }
    if (input.IsKeyPressed(Key::Num2))
    {
        SetViewmodel(false);
    }

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

    mnd::GameObject::Update(deltaTime);
}
