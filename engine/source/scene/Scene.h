/**
 * @file Scene.h
 * @brief Owns the game object tree and provides per-frame update + light collection.
 *
 * The Scene is the top-level container for all GameObjects. It owns them
 * via unique_ptr — GameObjects must be created through Scene::CreateObject
 * so that the scene always holds the authoritative lifetime.
 *
 * ## Relationship with Engine
 * The Engine holds one active Scene (Engine::SetScene / Engine::GetScene).
 * Each frame, Engine::Run() calls Scene-level queries to gather CameraData
 * and LightData before handing them to RenderQueue::Draw().
 *
 * ## Creating objects
 * @code
 *   Scene *scene = new Scene();
 *   engine.SetScene(scene);
 *
 *   auto *player = scene->CreateObject("Player");
 *   player->AddComponent(new PlayerControllerComponent());
 *
 *   auto *camera = scene->CreateObject("Camera", player);  // child of player
 *   camera->AddComponent(new CameraComponent());
 *   scene->SetMainCamera(camera);
 * @endcode
 *
 * @see GameObject, Engine::SetScene, CameraData, LightData
 */

#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "Common.h"
#include "Types.h"
#include "scene/GameObject.h"

namespace ENG
{

/**
 * @brief Container for the entire game object graph.
 *
 * Owns root-level GameObjects and provides the three services that
 * Engine::Run() needs each frame:
 * 1. Update() — propagate deltaTime to every active object and component.
 * 2. GetMainCamera() — return the designated camera object so the engine can
 *    extract CameraData.
 * 3. CollectLight() — walk the hierarchy and gather LightData from every
 *    LightComponent.
 */
class Scene
{
public:
    /**
     * @brief Update all root objects (which recursively update their children).
     * @param deltaTime Seconds since the previous frame.
     */
    void Update(f32 deltaTime);

    /// Destroy all objects and reset the scene to an empty state.
    void Clear();

    /**
     * @brief Create a plain GameObject owned by this scene.
     * @param name   Display name for the new object.
     * @param parent Optional parent; makes the object a child rather than a root.
     * @return Non-owning pointer to the new object.
     */
    GameObject *CreateObject(const std::string &name, GameObject *parent = nullptr);

    /**
     * @brief Create a typed GameObject subclass owned by this scene.
     *
     * Use this overload when you need to store extra data directly on the
     * GameObject instead of in a Component.
     *
     * @tparam T    A class derived from GameObject.
     * @param name  Display name.
     * @param parent Optional parent node.
     * @return Typed non-owning pointer to the new object.
     */
    template<typename T, typename = typename std::enable_if_t<std::is_base_of_v<GameObject, T>>>
    T *CreateObject(const str &name, GameObject *parent = nullptr)
    {
        auto obj = new T();
        obj->SetName(name);
        obj->m_scene = this;
        SetParent(obj, parent);

        return obj;
    }

    /**
     * @brief Move a GameObject in the hierarchy.
     * @param obj    The object to re-parent.
     * @param parent New parent, or nullptr to promote to root.
     * @return true if the operation succeeded.
     */
    bool SetParent(GameObject *obj, GameObject *parent);

    /**
     * @brief Designate the camera object whose CameraComponent drives the view.
     * @param camera A GameObject that must have a CameraComponent attached.
     */
    void SetMainCamera(GameObject *camera);

    /// Returns the current main camera object (set via SetMainCamera).
    GameObject *GetMainCamera();

    /**
     * @brief Walk the entire object tree and collect all LightData.
     *
     * Called by Engine::Run() once per frame. Each GameObject that has a
     * LightComponent contributes one LightData entry.
     *
     * @return Vector of LightData for all active lights in the scene.
     */
    std::vector<LightData> CollectLight();

private:
    /// Recursive DFS helper used by CollectLight().
    void CollectLightsRecursive(GameObject *obj, std::vector<LightData> &out);

private:
    std::vector<std::unique_ptr<GameObject>> m_objects;  ///< All root-level objects (own the tree).

    GameObject *m_mainCamera = nullptr;  ///< Non-owning pointer to the active camera object.
};

}  // namespace ENG
