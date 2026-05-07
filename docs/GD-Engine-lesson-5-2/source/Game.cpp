#include "Game.h"
#include "TestObject.h"
#include "Player.h"
#include "Bullet.h"
#include "JumpPlatform.h"

#include <iostream>

void Game::RegisterTypes()
{
    Player::Register();
    Bullet::Register();
    JumpPlatform::Register();
}

bool Game::Init()
{
    //auto scene = eng::Scene::Load("scenes/scene.sc");
    //m_scene = scene;
    //eng::Engine::GetInstance().SetScene(scene.get());

    m_scene = std::make_shared<eng::Scene>();
    eng::Engine::GetInstance().SetScene(m_scene.get());

    auto sprite = m_scene->CreateObject("Sprite");
    auto spriteComponent = new eng::SpriteComponent();

    auto texture = eng::Texture::Load("textures/brick.png");
    spriteComponent->SetTexture(texture);

    sprite->AddComponent(spriteComponent);
    sprite->SetPosition2D(glm::vec2(500.0f, 500.0f));

    spriteComponent->SetSize(glm::vec2(200.0f, 100.0f));
    spriteComponent->SetUpperRightUV(glm::vec2(2.0f, 1.0f));
    sprite->SetRotation2D(glm::radians(45.0f));


    auto camera = m_scene->CreateObject("Camera");
    auto cameraComponent = new eng::CameraComponent();
    camera->AddComponent(cameraComponent);
    m_scene->SetMainCamera(camera);

    return true;
}

void Game::Update(float deltaTime)
{
    m_scene->Update(deltaTime);
}

void Game::Destroy()
{

}