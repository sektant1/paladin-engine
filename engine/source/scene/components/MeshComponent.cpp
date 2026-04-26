#include "scene/components/MeshComponent.h"

#include "Engine.h"
#include "Log.h"
#include "render/Material.h"
#include "render/Mesh.h"
#include "render/RenderQueue.h"
#include "scene/GameObject.h"

namespace mnd
{

MeshComponent::MeshComponent(const std::shared_ptr<Material> &material, const std::shared_ptr<Mesh> &mesh)
    : m_material(material)
    , m_mesh(mesh)
{
}

void MeshComponent::Update(f32 deltaTime)
{
    if (!m_material || !m_mesh)
    {
        const char *ownerName = GetOwner() ? GetOwner()->GetName().c_str() : "<no-owner>";
        LOG_WARN(
            "MeshComponent on '%s' missing %s%s%s",
            ownerName,
            m_material ? "" : "material",
            (!m_material && !m_mesh) ? " and " : "",
            m_mesh ? "" : "mesh"
        );
        return;
    }

    RenderCommand command;
    command.material    = m_material.get();
    command.mesh        = m_mesh.get();
    command.modelMatrix = GetOwner()->GetWorldTransform();

    auto &renderQueue = Engine::GetInstance().GetRenderQueue();
    renderQueue.Submit(command);
}

void MeshComponent::SetMaterial(const std::shared_ptr<Material> &material)
{
    m_material = material;
}

void MeshComponent::SetMesh(const std::shared_ptr<Mesh> &mesh)
{
    m_mesh = mesh;
}

void MeshComponent::LoadProperties(const nlohmann::json &json)
{
    // Material
    if (json.contains("material"))
    {
        auto     &matObj = json["material"];
        const str path   = matObj.value("path", "");
        auto      mat    = Material::Load(path);
        if (mat && matObj.contains("params"))
        {
            auto &paramsObj = matObj["params"];

            // Float
            if (paramsObj.contains("float"))
            {
                for (auto &param : paramsObj["float"])
                {
                    str name  = param.value("name", "");
                    f32 value = param.value("value", 0.0f);
                    mat->SetParam(name, value);
                }
            }

            // Float 2
            if (paramsObj.contains("float2"))
            {
                for (auto &param : paramsObj["float2"])
                {
                    str name = param.value("name", "");
                    f32 v0   = param.value("value0", 0.0F);
                    f32 v1   = param.value("value1", 0.0F);

                    mat->SetParam(name, v0, v1);
                }
            }

            // Float 3
            if (paramsObj.contains("float3"))
            {
                for (auto &param : paramsObj["float3"])
                {
                    str name = param.value("name", "");
                    f32 v0   = param.value("value0", 0.0F);
                    f32 v1   = param.value("value1", 0.0F);
                    f32 v2   = param.value("value2", 0.0F);

                    mat->SetParam(name, vec3(v0, v1, v2));
                }
            }

            // Float 4
            if (paramsObj.contains("float4"))
            {
                for (auto &param : paramsObj["float4"])
                {
                    str name = param.value("name", "");
                    f32 v0   = param.value("value0", 0.0F);
                    f32 v1   = param.value("value1", 0.0F);
                    f32 v2   = param.value("value2", 0.0F);
                    f32 v3   = param.value("value3", 0.0F);

                    mat->SetParam(name, vec4(v0, v1, v2, v3));
                }
            }

            // Textures
            if (paramsObj.contains("textures"))
            {
                for (auto &param : paramsObj["textures"])
                {
                    str  name    = param.value("name", "");
                    str  texPath = param.value("path", "");
                    auto texture = Texture::Load(texPath);

                    mat->SetParam(name, texture);
                }
            }
        }

        SetMaterial(mat);
    }

    // Mesh
    if (json.contains("mesh"))
    {
        const auto       &mesh = json["mesh"];
        const std::string type = mesh.value("type", "box");
        if (type == "box")
        {
            vec3 extents(mesh.value("x", 1.0F), mesh.value("y", 1.0F), mesh.value("z", 1.0F));
            auto mesh = Mesh::CreateBox(extents);
            SetMesh(mesh);
        } else if (type == "sphere")
        {
            f32  r    = mesh.value("r", 1.0F);
            auto mesh = Mesh::CreateSphere(r, 16, 16);
            SetMesh(mesh);
        }
    }
}

}  // namespace mnd
