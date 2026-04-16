#pragma once

#include <memory>

#include "ENG.h"

class Lab : public ENG::Application
{
public:
    bool Init() override;
    void Update(ENG::f32 deltaTime) override;
    void Destroy() override;

private:
    ENG::Material              m_material;
    std::unique_ptr<ENG::Mesh> m_mesh;
    float                      m_offsetX = 0.0F;
    float                      m_offsetY = 0.0F;
};
