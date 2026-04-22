#include <memory>

#include "Game.h"

#include "Engine.h"
#include "GLFW/glfw3.h"
#include "TestObject.h"
#include "Types.h"
#include "render/Builder.h"
#include "render/Material.h"
#include "scene/GameObject.h"
#include "scene/Scene.h"
#include "scene/components/AnimationComponent.h"
#include "scene/components/CameraComponent.h"
#include "scene/components/LightComponent.h"
#include "scene/components/MeshComponent.h"
#include "scene/components/PlayerControllerComponent.h"

bool Game::Init()
{
    LOG_INFO("Game::Init");

    auto &fs      = ENG::Engine::GetInstance().GetFileSystem();
    auto  texture = ENG::Texture::Load("textures/brick.png");

    m_scene = new ENG::Scene();

    ENG::Engine::GetInstance().SetScene(m_scene);

    m_mainCamera = m_scene->CreateObject("Main Camera");
    m_mainCamera->AddComponent(new ENG::CameraComponent());
    m_mainCamera->SetPosition(ENG::vec3(0.0F, 0.0F, 2.0F));
    m_mainCamera->AddComponent(new ENG::PlayerControllerComponent());

    m_altCamera = m_scene->CreateObject("Alt Camera");
    m_altCamera->AddComponent(new ENG::CameraComponent());
    m_altCamera->SetPosition(ENG::vec3(4.0F, 2.0F, 4.0F));
    m_altCamera->AddComponent(new ENG::PlayerControllerComponent());

    m_scene->SetMainCamera(m_mainCamera);

    m_scene->CreateObject<TestObject>("TestObject");

    auto material = ENG::Material::Load("materials/brick.mat");

    // Build cube once -> single GPU upload. Shared across all instances below.
    auto mesh = ENG::Mesh::CreateCube();

    auto objectB = m_scene->CreateObject("ObjectB");
    objectB->AddComponent(new ENG::MeshComponent(material, mesh));
    objectB->SetPosition(ENG::vec3(1.0F, 2.0F, 2.0F));
    objectB->SetRotation(ENG::vec3(0.0F, 2.0F, 0.0F));

    // Orbit system: child parented to parent. Rotating parent -> child orbits.
    m_orbitParent = m_scene->CreateObject("OrbitParent");
    m_orbitParent->AddComponent(new ENG::MeshComponent(material, mesh));
    m_orbitParent->SetPosition(ENG::vec3(3.0F, 0.0F, -3.0F));
    m_orbitParent->SetScale(ENG::vec3(1.0F));

    auto moon = m_scene->CreateObject("Moon", m_orbitParent);
    moon->AddComponent(new ENG::MeshComponent(material, mesh));
    moon->SetPosition(ENG::vec3(2.5F, 0.0F, 0.0F));  // orbit radius in parent-local space
    moon->SetScale(ENG::vec3(0.3F));

    // Planet: slow self-rotation on tilted axis -> continuous quat accumulation.
    m_planet = m_scene->CreateObject("Planet");
    m_planet->AddComponent(new ENG::MeshComponent(material, mesh));
    m_planet->SetPosition(ENG::vec3(-4.0F, 1.0F, -2.0F));
    m_planet->SetScale(ENG::vec3(1.2F));

    auto objectC = m_scene->CreateObject("ObjectC");
    objectC->AddComponent(new ENG::MeshComponent(material, mesh));
    objectC->SetPosition(ENG::vec3(-2.0F, 0.0F, 0.0F));
    objectC->SetRotation(ENG::vec3(1.0F, 0.0F, -5.0F));
    objectC->SetScale(ENG::vec3(1.5F, 1.5F, 1.5F));

    // auto suzanneMesh     = ENG::Mesh::Load("models/suzanne/Suzanne.gltf");
    // auto suzanneMaterial = ENG::Material::Load("materials/suzanne.mat");
    // auto suzanneObj      = m_scene->CreateObject("Suzanne");

    // suzanneObj->AddComponent(new ENG::MeshComponent(suzanneMaterial, suzanneMesh));
    // suzanneObj->SetPosition(ENG::vec3(0.0F, 0.0F, -5.0F));

    auto suzanneObject = ENG::GameObject::LoadGLTF("models/suzanne/Suzanne.gltf");
    suzanneObject->SetPosition(ENG::vec3(0.0F, 0.0F, -5.0F));

    auto gun = ENG::GameObject::LoadGLTF("models/carbine/scene.gltf");
    gun->SetParent(m_mainCamera);
    gun->SetPosition(ENG::vec3(0.75f, -0.5f, -0.75f));
    gun->SetScale(ENG::vec3(-1.0f, 1.0f, 1.0f));

    if (auto anim = gun->GetComponent<ENG::AnimationComponent>())
    {
        if (auto bullet = gun->FindChildByName("bullet_33"))
        {
            bullet->SetActive(false);
        }

        if (auto fire = gun->FindChildByName("BOOM_35"))
        {
            fire->SetActive(false);
        }

        anim->Play("shoot", false);
    }

    auto light          = m_scene->CreateObject("Light");
    auto lightComponent = new ENG::LightComponent();
    lightComponent->SetColor(ENG::vec3(1.0f));
    light->AddComponent(lightComponent);
    light->SetPosition(ENG::vec3(0.0f, 5.0f, 0.0f));

    return true;
}

void Game::Update(float deltaTime)
{
    m_scene->Update(deltaTime);

    auto &inputManager = ENG::Engine::GetInstance().GetInputManager();

    bool toggleKeyNow = inputManager.IsKeyPressed(GLFW_KEY_C);
    if (toggleKeyNow && !m_toggleKeyPrev)
    {
        auto *current = m_scene->GetMainCamera();
        m_scene->SetMainCamera(current == m_mainCamera ? m_altCamera : m_mainCamera);
    }
    m_toggleKeyPrev = toggleKeyNow;

    // Spin only parent. Child inherits world transform -> orbits.
    m_orbitAngle += deltaTime * 1.5F;
    m_orbitParent->SetRotation(ENG::angleAxis(m_orbitAngle, ENG::vec3(0.0F, 1.0F, 0.0F)));

    // Planet: accumulate rotation via quat multiplication each frame.
    // delta = small quat about tilted axis. newRot = delta * oldRot. Normalize -> drift-free.
    ENG::vec3 tiltedAxis = normalize(ENG::vec3(0.3F, 1.0F, 0.1F));
    ENG::quat delta      = ENG::angleAxis(deltaTime * 0.3F, tiltedAxis);
    m_planet->SetRotation(normalize(delta * m_planet->GetRotation()));
}

void Game::Destroy()
{
    LOG_INFO("Game::Destroy");
}
