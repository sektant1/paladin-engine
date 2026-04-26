#include "scene/components/SkinnedMeshComponent.h"

#include "Engine.h"
#include "Log.h"
#include "animation/Skeleton.h"
#include "render/Material.h"
#include "render/Mesh.h"
#include "render/RenderQueue.h"
#include "scene/GameObject.h"
#include "scene/components/AnimationComponent.h"

namespace mnd
{

SkinnedMeshComponent::SkinnedMeshComponent(const std::shared_ptr<Material> &material,
                                           const std::shared_ptr<Mesh>     &mesh,
                                           const std::shared_ptr<Skeleton> &skeleton)
    : m_material(material)
    , m_mesh(mesh)
    , m_skeleton(skeleton)
{
    if (m_skeleton)
    {
        m_identityPalette.assign(m_skeleton->Size(), glm::mat4(1.0f));
    }
}

void SkinnedMeshComponent::Update(f32 deltaTime)
{
    if (!m_material || !m_mesh)
    {
        return;
    }

    const std::vector<glm::mat4> *palette = &m_identityPalette;
    if (m_paletteSource)
    {
        const auto &live = m_paletteSource->GetPalette();
        if (!live.empty())
        {
            palette = &live;
        }
    }

    if (!palette->empty())
    {
        m_material->SetParam("uBones", *palette);
    }

    RenderCommand command;
    command.material    = m_material.get();
    command.mesh        = m_mesh.get();
    command.modelMatrix = GetOwner()->GetWorldTransform();

    Engine::GetInstance().GetRenderQueue().Submit(command);
}

void SkinnedMeshComponent::SetMaterial(const std::shared_ptr<Material> &material)
{
    m_material = material;
}

void SkinnedMeshComponent::SetMesh(const std::shared_ptr<Mesh> &mesh)
{
    m_mesh = mesh;
}

void SkinnedMeshComponent::SetSkeleton(const std::shared_ptr<Skeleton> &skeleton)
{
    m_skeleton = skeleton;
    if (m_skeleton)
    {
        m_identityPalette.assign(m_skeleton->Size(), glm::mat4(1.0f));
    }
}

void SkinnedMeshComponent::SetPaletteSource(AnimationComponent *anim)
{
    m_paletteSource = anim;
}

}  // namespace mnd
