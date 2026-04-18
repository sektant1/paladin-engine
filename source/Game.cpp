#include "Game.h"

#include "Engine.h"
#include "LabObject.h"
#include "TestObject.h"
#include "scene/Scene.h"
#include "scene/components/CameraComponent.h"

bool Game::Init()
{
    LOG_INFO("Game::Init");

    m_scene = new ENG::Scene();

    auto camera = m_scene->CreateObject("Camera");
    camera->AddComponent(new ENG::CameraComponent());
    camera->SetPosition(ENG::vec3(0.0F, 0.0F, 2.0F));

    m_scene->SetMainCamera(camera);

    // m_scene->CreateObject<LabObject>("LabObject");
    m_scene->CreateObject<TestObject>("TestObject");

    ENG::Engine::GetInstance().SetScene(m_scene);
    return true;
}

void Game::Update(float deltaTime)
{
    m_scene->Update(deltaTime);
}

void Game::Destroy()
{
    LOG_INFO("Game::Destroy");
}
