#include <array>
#include <cstdint>
#include <functional>

#include "Game.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/quaternion.hpp>
#include <imgui.h>

#include "Bullet.h"
#include "Enemy.h"
#include "GameConstants.h"
#include "JumpPlatform.h"
#include "Monad.h"
#include "Player.h"
#include "physics/Raycast.h"

bool Game::Init()
{
    LOG_INFO("Game::Init");

    auto scene = mnd::Scene::Load(kTestScenePath);
    m_scene    = scene;
    mnd::Engine::GetInstance().SetScene(scene.get());

    RegisterDebugPanels();

    return true;
}

void Game::RegisterTypes()
{
    Player::Register();
    Enemy::Register();
    JumpPlatform::Register();
    Bullet::Register();
}

void Game::Update(mnd::f32 deltaTime)
{
    m_scene->Update(deltaTime);
    auto &inputManager = mnd::Engine::GetInstance().GetInputManager();
    if (inputManager.IsKeyPressed(mnd::Key::Escape))
    {
        SetNeedsToBeClosed(true);
    }
}

void Game::Destroy()
{
    LOG_INFO("Game::Destroy");
}

void Game::RegisterDebugPanels()
{
    auto &editor = mnd::Engine::GetInstance().GetEditor();

    editor.RegisterPanel(
        "RNG",
        [this]
        {
            ImGui::Text("Root seed: 0x%016llX", static_cast<unsigned long long>(m_rng.Seed()));

            static std::uint64_t seedField = 0xC0FFEE0123456789ULL;
            ImGui::InputScalar(
                "seed", ImGuiDataType_U64, &seedField, nullptr, nullptr, "%016llX", ImGuiInputTextFlags_CharsHexadecimal
            );
            if (ImGui::Button("Reseed"))
            {
                m_rng.Reseed(seedField);
            }

            ImGui::Separator();
            ImGui::TextUnformatted("Histogram (loot stream, 1000 rolls in [0,9])");
            static std::array<int, 10> histogram {};
            if (ImGui::Button("Roll 1000"))
            {
                histogram.fill(0);
                for (int i = 0; i < 1000; ++i)
                {
                    ++histogram[static_cast<std::size_t>(m_rng.loot.Range(0, 9))];
                }
            }
            for (int i = 0; i < static_cast<int>(histogram.size()); ++i)
            {
                ImGui::Text("%d: %4d", i, histogram[static_cast<std::size_t>(i)]);
            }

            ImGui::Separator();
            ImGui::TextUnformatted("Single rolls");
            ImGui::Text("layout.Range(0,99)  = %d", m_rng.layout.Range(0, 99));
            ImGui::Text("ai.Float01()        = %.4f", static_cast<double>(m_rng.ai.Float01()));
            ImGui::Text("fx.FloatRange(-1,1) = %.4f", static_cast<double>(m_rng.fx.FloatRange(-1.0F, 1.0F)));
        }
    );

    editor.RegisterPanel(
        "Health",
        []
        {
            auto *scene = mnd::Engine::GetInstance().GetScene();
            if (scene == nullptr)
            {
                ImGui::TextUnformatted("No active scene.");
                return;
            }

            std::function<void(mnd::GameObject *)> visit;
            visit = [&](mnd::GameObject *obj)
            {
                if (obj == nullptr)
                {
                    return;
                }
                if (auto *health = obj->GetComponent<mnd::HealthComponent>())
                {
                    ImGui::PushID(obj);
                    ImGui::Text(
                        "%s  %d / %d%s",
                        obj->GetName().c_str(),
                        health->GetCurrent(),
                        health->GetMax(),
                        health->IsDead() ? "  [DEAD]" : ""
                    );
                    ImGui::SameLine();
                    if (ImGui::SmallButton("dmg10"))
                    {
                        mnd::DamageInfo info;
                        info.amount = 10.0F;
                        health->ApplyDamage(info);
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("kill"))
                    {
                        mnd::DamageInfo info;
                        info.amount = static_cast<float>(health->GetCurrent());
                        health->ApplyDamage(info);
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("revive"))
                    {
                        health->Revive();
                    }
                    ImGui::PopID();
                }
                for (const auto &child : obj->GetChildren())
                {
                    visit(child.get());
                }
            };

            for (const auto &root : scene->GetRootObjects())
            {
                visit(root.get());
            }
        }
    );

    editor.RegisterPanel(
        "Raycast",
        []
        {
            auto *scene = mnd::Engine::GetInstance().GetScene();
            if (scene == nullptr)
            {
                ImGui::TextUnformatted("No active scene.");
                return;
            }
            auto *camera = scene->GetMainCamera();
            if (camera == nullptr)
            {
                ImGui::TextUnformatted("No main camera set.");
                return;
            }

            const auto      origin   = camera->GetWorldPosition();
            const auto      rotation = camera->GetWorldRotation();
            const glm::vec3 forward  = glm::normalize(rotation * glm::vec3(0.0F, 0.0F, -1.0F));

            static float maxDistance = 100.0F;
            ImGui::SliderFloat("range", &maxDistance, 1.0F, 500.0F);

            const glm::vec3 endPoint = origin + forward * maxDistance;
            ImGui::Text(
                "origin  = (%.2f, %.2f, %.2f)",
                static_cast<double>(origin.x),
                static_cast<double>(origin.y),
                static_cast<double>(origin.z)
            );
            ImGui::Text(
                "forward = (%.2f, %.2f, %.2f)",
                static_cast<double>(forward.x),
                static_cast<double>(forward.y),
                static_cast<double>(forward.z)
            );

            mnd::RayHit hit;
            const bool  didHit = mnd::Engine::GetInstance().GetPhysicsManager().Raycast(origin, endPoint, hit);

            ImGui::Separator();
            if (didHit)
            {
                ImGui::Text(
                    "HIT  fraction=%.3f  distance=%.2f",
                    static_cast<double>(hit.fraction),
                    static_cast<double>(hit.fraction * maxDistance)
                );
                ImGui::Text(
                    "point  = (%.2f, %.2f, %.2f)",
                    static_cast<double>(hit.point.x),
                    static_cast<double>(hit.point.y),
                    static_cast<double>(hit.point.z)
                );
                ImGui::Text(
                    "normal = (%.2f, %.2f, %.2f)",
                    static_cast<double>(hit.normal.x),
                    static_cast<double>(hit.normal.y),
                    static_cast<double>(hit.normal.z)
                );
                ImGui::Text("object = %s", hit.object != nullptr ? hit.object->GetName().c_str() : "<no owner>");
            } else
            {
                ImGui::TextUnformatted("no hit");
            }
        }
    );
}
