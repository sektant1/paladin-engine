#include "scene/components/MeshComponent.h"
#include "render/Material.h"
#include "render/Mesh.h"
#include "render/RenderQueue.h"
#include "scene/GameObject.h"
#include "Engine.h"

namespace eng
{
    MeshComponent::MeshComponent(const std::shared_ptr<Material>& material, const std::shared_ptr<Mesh>& mesh)
        : m_material(material), m_mesh(mesh)
    {
    }

    void MeshComponent::LoadProperties(const nlohmann::json& json)
    {
        if (json.contains("material"))
        {
            auto& matObj = json["material"];
            const std::string path = matObj.value("path", "");
            auto mat = Material::Load(path);
            if (mat && matObj.contains("params"))
            {
                auto& paramsObj = matObj["params"];

                // Floats
                if (paramsObj.contains("float"))
                {
                    for (auto& p : paramsObj["float"])
                    {
                        std::string name = p.value("name", "");
                        float value = p.value("value", 0.0f);
                        mat->SetParam(name, value);
                    }
                }

                // Float2
                if (paramsObj.contains("float2"))
                {
                    for (auto& p : paramsObj["float2"])
                    {
                        std::string name = p.value("name", "");
                        float v0 = p.value("value0", 0.0f);
                        float v1 = p.value("value1", 0.0f);
                        mat->SetParam(name, v0, v1);
                    }
                }

                // Float3
                if (paramsObj.contains("float3"))
                {
                    for (auto& p : paramsObj["float3"])
                    {
                        std::string name = p.value("name", "");
                        float v0 = p.value("value0", 0.0f);
                        float v1 = p.value("value1", 0.0f);
                        float v2 = p.value("value2", 0.0f);
                        mat->SetParam(name, glm::vec3(v0, v1, v2));
                    }
                }

                // Textures
                if (paramsObj.contains("textures"))
                {
                    for (auto& p : paramsObj["textures"])
                    {
                        std::string name = p.value("name", "");
                        std::string texPath = p.value("path", "");
                        auto texture = Texture::Load(texPath);

                        mat->SetParam(name, texture);
                    }
                }
            }
            SetMaterial(mat);
        }

        if (json.contains("mesh"))
        {
            auto& meshObj = json["mesh"];
            const std::string type = meshObj.value("type", "box");
            if (type == "box")
            {
                glm::vec3 extents;
                extents.x = meshObj.value("x", 1.0f);
                extents.y = meshObj.value("y", 1.0f);
                extents.z = meshObj.value("z", 1.0f);
                auto mesh = Mesh::CreateBox(extents);
                SetMesh(mesh);
            }
            else if (type == "sphere")
            {
                float r = meshObj.value("r", 1.0f);
                auto mesh = Mesh::CreateSphere(r, 16, 16);
                SetMesh(mesh);
            }
        }
    }

    void MeshComponent::Update(float deltaTime)
    {
        if (!m_material || !m_mesh)
        {
            return;
        }

        RenderCommand command;
        command.material = m_material.get();
        command.mesh = m_mesh.get();
        command.modelMatrix = GetOwner()->GetWorldTransform();

        auto& renderQueue = Engine::GetInstance().GetRenderQueue();
        renderQueue.Submit(command);
    }

    void MeshComponent::SetMaterial(const std::shared_ptr<Material>& material)
    {
        m_material = material;
    }

    void MeshComponent::SetMesh(const std::shared_ptr<Mesh>& mesh)
    {
        m_mesh = mesh;
    }
}