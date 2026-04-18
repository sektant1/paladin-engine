#pragma once

#include <memory>

#include "Types.h"
#include "scene/Component.h"

namespace ENG
{

class Material;
class Mesh;

class MeshComponent : public Component
{
    COMPONENT(MeshComponent)
public:
    MeshComponent(const std::shared_ptr<Material> &material, const std::shared_ptr<Mesh> &mesh);
    void Update(f32 deltaTime) override;

private:
    std::shared_ptr<Material> m_material;
    std::shared_ptr<Mesh>     m_mesh;
};
}  // namespace ENG
