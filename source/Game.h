#pragma once

#include "ENG.h"

class Game : public ENG::Application
{
public:
    bool Init() override;
    void Update(float deltaTime) override;
    void Destroy() override;

private:
    ENG::Scene *m_scene = nullptr;
};
