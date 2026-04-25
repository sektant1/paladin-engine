#include "Game.h"

#include <GLFW/glfw3.h>

#include "COA.h"
#include "GameConstants.h"
#include "Player.h"

bool Game::Init()
{
    LOG_INFO("Game::Init");

    auto scene = COA::Scene::Load(kInitialScenePath);
    m_scene    = scene;
    COA::Engine::GetInstance().SetScene(scene.get());

    return true;
}

void Game::RegisterTypes()
{
    Player::Register();
}

void Game::Update(COA::f32 deltaTime)
{
    m_scene->Update(deltaTime);
    auto &inputManager = COA::Engine::GetInstance().GetInputManager();
    if (inputManager.IsKeyPressed(COA::Key::Escape))
    {
        SetNeedsToBeClosed(true);
    }
}

void Game::Destroy()
{
    LOG_INFO("Game::Destroy");
}
