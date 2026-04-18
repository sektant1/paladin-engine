#include "Game.h"
#include "TestObject.h"


bool Game::Init()
{
    m_scene = new eng::Scene();

    auto camera = m_scene->CreateObject("Camera");
    camera->AddComponent(new eng::CameraComponent());
    camera->SetPosition(glm::vec3(0.0f, 0.0f, 2.0f));

    m_scene->SetMainCamera(camera);

    m_scene->CreateObject<TestObject>("TestObject");
    
    eng::Engine::GetInstance().SetScene(m_scene);
    
    return true;
}

void Game::Update(float deltaTime)
{
    m_scene->Update(deltaTime);
}

void Game::Destroy()
{

}