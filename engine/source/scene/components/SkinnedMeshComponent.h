/**
 * @file SkinnedMeshComponent.h
 * @ingroup mnd_components
 * @brief Renderable component for GPU-skinned meshes driven by a Skeleton.
 *
 * Mirrors MeshComponent but uploads a per-frame bone palette as the
 * `uBones` mat4[] uniform on its Material before submitting the draw.
 *
 * The palette source is plugged in externally — typically by an
 * AnimationComponent on an ancestor GameObject. Until a source is wired,
 * the component renders the bind pose (identity palette).
 */

#pragma once

#include <memory>
#include <vector>

#include <glm/mat4x4.hpp>

#include "Types.h"
#include "scene/Component.h"

namespace mnd
{

class Material;
class Mesh;
class Skeleton;
class AnimationComponent;

class SkinnedMeshComponent : public Component
{
    COMPONENT(SkinnedMeshComponent)

public:
    SkinnedMeshComponent() = default;
    SkinnedMeshComponent(const std::shared_ptr<Material> &material,
                         const std::shared_ptr<Mesh>     &mesh,
                         const std::shared_ptr<Skeleton> &skeleton);

    void Update(f32 deltaTime) override;

    void SetMaterial(const std::shared_ptr<Material> &material);
    void SetMesh(const std::shared_ptr<Mesh> &mesh);
    void SetSkeleton(const std::shared_ptr<Skeleton> &skeleton);

    /// Plug an AnimationComponent (typically on the model root) as the live palette source.
    void SetPaletteSource(AnimationComponent *anim);

private:
    std::shared_ptr<Material> m_material;
    std::shared_ptr<Mesh>     m_mesh;
    std::shared_ptr<Skeleton> m_skeleton;
    AnimationComponent       *m_paletteSource = nullptr;
    std::vector<glm::mat4>    m_identityPalette;  ///< Bind-pose fallback when no source wired.
};

}  // namespace mnd
