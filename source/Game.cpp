#include "Game.h"

#include "LabObject.h"

// #include "TestObject.h"

bool Game::Init()
{
    // m_scene.CreateObject<TestObject>("TestObject");
    m_scene.CreateObject<LabObject>("LabObject");
    return true;
}

void Game::Update(float deltaTime)
{
    m_scene.Update(deltaTime);
}

void Game::Destroy() {}
