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
        MeshComponent() = default;
        MeshComponent(const std::shared_ptr<Material>& material, const std::shared_ptr<Mesh>& mesh);
        void LoadProperties(const nlohmann::json& json) override;
        void Update(float deltaTime) override;

        void SetMaterial(const std::shared_ptr<Material>& material);
        void SetMesh(const std::shared_ptr<Mesh>& mesh);

    private:
        std::shared_ptr<Material> m_material;
        std::shared_ptr<Mesh> m_mesh;
    };
}