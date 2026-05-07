#pragma once
#include <eng.h>
#include <memory>

class Game : public eng::Application
{
public:
    void RegisterTypes() override;
    bool Init() override;
    void Update(float deltaTime) override;
    void Destroy() override;

private:
    std::shared_ptr<eng::Scene> m_scene;
};