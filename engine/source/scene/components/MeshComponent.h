/**
 * @file MeshComponent.h
 * @ingroup coa_components
 * @brief Bridges the scene graph with the render pipeline by submitting a RenderCommand each frame.
 *
 * Attaching a MeshComponent to a GameObject makes it visible. On each
 * Update() call the component reads the owner's world transform, packages
 * it together with its Mesh and Material into a RenderCommand, and submits
 * it to the engine's RenderQueue.
 *
 * ## Ownership model
 * Both Mesh and Material are shared via shared_ptr — multiple MeshComponents
 * (and multiple GameObjects) can reference the same GPU resources without
 * duplication. The RenderCommand holds only raw pointers for the duration of
 * the frame; it does not extend lifetime.
 *
 * @see RenderCommand, RenderQueue::Submit, Material, Mesh
 */

#pragma once

#include <memory>

#include "Types.h"
#include "scene/Component.h"

namespace COA
{

class Material;
class Mesh;

/**
 * @brief Makes a GameObject renderable by submitting its mesh each frame.
 *
 * Update() packages (Mesh + Material + owner world transform) into a
 * RenderCommand and hands it to the engine's RenderQueue. The actual draw
 * call happens later in RenderQueue::Draw().
 */
class MeshComponent : public Component
{
    COMPONENT(MeshComponent)
public:
    /**
     * @brief Construct a MeshComponent with a mesh-material pair.
     * @param material The surface description (shader + uniforms + textures).
     * @param mesh     The GPU geometry to draw.
     */
    MeshComponent(const std::shared_ptr<Material> &material, const std::shared_ptr<Mesh> &mesh);

    /**
     * @brief Submit a RenderCommand for this object's current world transform.
     * @param deltaTime Not used directly, but keeps the Component interface uniform.
     */
    void Update(f32 deltaTime) override;

private:
    std::shared_ptr<Material> m_material;  ///< Shared surface material (shader + params).
    std::shared_ptr<Mesh>     m_mesh;      ///< Shared GPU geometry.
};

}  // namespace COA
