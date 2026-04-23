/**
 * @file LightComponent.h
 * @ingroup coa_components
 * @brief Tags a GameObject as a point light source in the scene.
 *
 * When a GameObject has a LightComponent, Scene::CollectLight() picks it up
 * during the per-frame traversal and adds a LightData entry (world position
 * + RGB colour) to the list forwarded to RenderQueue::Draw(). The queue then
 * uploads the light data as uniforms to every material that is drawn.
 *
 * ## Usage
 * @code
 *   auto *lightObj = scene->CreateObject("SunLight");
 *   lightObj->SetPosition(vec3(10, 20, 5));
 *   auto *light = new LightComponent();
 *   light->SetColor(vec3(1.0f, 0.95f, 0.8f));   // warm white
 *   lightObj->AddComponent(light);
 * @endcode
 *
 * @note Only the owner's world position is used — there is no direction,
 *       radius, or attenuation yet (point light model).
 *
 * @see LightData, Scene::CollectLight, RenderQueue::Draw
 */

#pragma once
#include "scene/Component.h"

namespace COA
{

/**
 * @brief Point light that contributes position + colour to the render pass.
 *
 * The world-space position is read from the owner GameObject's transform.
 * Multiple LightComponents in the same scene are all collected and passed
 * to every shader as a uniform array.
 */
class LightComponent : public Component
{
    COMPONENT(LightComponent)
public:
    /// No per-frame computation needed; position is queried from owner on demand.
    void Update(f32 deltaTime) override;

    /// Returns the linear RGB colour of this light (default: white {1,1,1}).
    const vec3 &GetColor() const;

    /**
     * @brief Set the light's emission colour.
     * @param color Linear RGB values in [0, 1] (HDR values > 1 are allowed).
     */
    void SetColor(const vec3 &color);

private:
    vec3 m_color = vec3(1.0F);  ///< Emission colour; default is full-intensity white.
};

}  // namespace COA
