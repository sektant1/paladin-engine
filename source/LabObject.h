#pragma once

#include "ENG.h"

class LabObject : public ENG::GameObject
{
public:
    LabObject();

    void Update(ENG::f32 deltaTime) override;

private:
    std::shared_ptr<ENG::Material> m_material;

    float m_time      = 0.0F;
    float m_timeScale = 0.3F;
};
