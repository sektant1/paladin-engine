#pragma once

#include "COA.h"

class Game : public COA::Application
{
public:
    bool Init() override;
    void Update(float deltaTime) override;
    void Destroy() override;

private:
    COA::Scene      *m_scene          = nullptr;
    COA::GameObject *m_mainCamera     = nullptr;
    COA::GameObject *m_altCamera      = nullptr;
    bool             m_toggleKeyPrev  = false;
    COA::GameObject *m_orbitParent    = nullptr;
    float            m_orbitAngle     = 0.0F;
    COA::GameObject *m_planet         = nullptr;
};
