#pragma once

#include "COA.h"

class LabObject : public COA::GameObject
{
public:
    LabObject();

    void Update(COA::f32 deltaTime) override;

private:
    std::shared_ptr<COA::Material> m_material;

    float m_time      = 0.0F;
    float m_timeScale = 0.3F;
};
