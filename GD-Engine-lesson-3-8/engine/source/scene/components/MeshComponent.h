#pragma once

#include "scene/Component.h"
#include <memory>

namespace eng
{
    class Material;
    class Mesh;

    class MeshComponent : public Component
    {
        COMPONENT(MeshComponent)
    public:
        MeshComponent(const std::shared_ptr<Material>& material, const std::shared_ptr<Mesh>& mesh);
        void Update(float deltaTime) override;

    private:
        std::shared_ptr<Material> m_material;
        std::shared_ptr<Mesh> m_mesh;
    };
}