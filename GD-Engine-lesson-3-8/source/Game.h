#pragma once
#include <eng.h>
#include <memory>

class Game : public eng::Application
{
public:
    bool Init() override;
    void Update(float deltaTime) override;
    void Destroy() override;

private:
    eng::Scene* m_scene = nullptr;
};