/**
 * @file GameObject.h
 * @ingroup coa_scene
 * @brief Scene node with a 3D transform and an attachable component list.
 *
 * ## Scene graph overview
 * GameObjects form a tree. Each node stores a local transform (position,
 * rotation, scale) relative to its parent. World-space values are computed
 * by concatenating transforms up to the root.
 *
 * ```
 *   Scene
 *   └── PlayerObject  (has PlayerControllerComponent, CameraComponent)
 *       └── GunObject (has MeshComponent, AnimationComponent)
 * ```
 *
 * ## Component lookup
 * Components are stored by pointer inside the object. Use GetComponent<T>()
 * for typed access:
 * @code
 *   auto *cam = player->GetComponent<CameraComponent>();
 *   if (cam) cam->DoSomething();
 * @endcode
 *
 * ## GLTF loading
 * LoadGLTF() creates a GameObject hierarchy that mirrors the node tree of a
 * .glb/.gltf file, attaching MeshComponent and AnimationComponent as needed.
 *
 * @see Component, Scene, MeshComponent, CameraComponent
 */

#pragma once
#include <memory>
#include <vector>

#include "Types.h"
#include "scene/Component.h"

namespace COA
{
class Scene;

struct ObjectCreatorBase
{
    virtual ~ObjectCreatorBase()           = default;
    virtual GameObject *CreateGameObject() = 0;
};

template<typename T>
struct ObjectCreator : ObjectCreatorBase
{
    GameObject *CreateGameObject() override { return new T(); }
};

class GameObjectFactory
{
public:
    static GameObjectFactory &GetInstance()
    {
        static GameObjectFactory instance;
        return instance;
    }

    template<typename T>
    void RegisterObject(const std::string &name)
    {
        m_creators.emplace(name, std::make_unique<ObjectCreator<T>>());
    }

    GameObject *CreateGameObject(const std::string &typeName)
    {
        auto iter = m_creators.find(typeName);
        if (iter == m_creators.end())
        {
            return nullptr;
        }
        return iter->second->CreateGameObject();
    }

private:
    std::unordered_map<std::string, std::unique_ptr<ObjectCreatorBase>> m_creators;
};

/**
 * @brief Node in the scene graph: a named transform that owns components and children.
 *
 * GameObjects are created and owned exclusively by Scene (via Scene::CreateObject).
 * They are never constructed directly to ensure the scene always holds ownership.
 */
class GameObject
{
public:
    virtual ~GameObject() = default;

    virtual void LoadProperties(const nlohmann::json &json) {}

    virtual void Init() {}

    /**
     * @brief Update this object and all its components and children recursively.
     * @param deltaTime Seconds since the previous frame.
     */
    virtual void Update(f32 deltaTime);

    const str &GetName() const;           ///< Returns the object's display name.
    void       SetName(const str &name);  ///< Sets the display name (used by FindChildByName).

    GameObject *GetParent();  ///< Returns the parent node, or nullptr for root objects.

    bool IsAlive() const;   ///< Returns false after MarkForDestroy() is called.
    void MarkForDestroy();  ///< Flag this object for removal at end of frame.

    /**
     * @brief Attach a component to this object (takes ownership).
     * @param component Heap-allocated Component subclass. The GameObject owns it.
     */
    void AddComponent(Component *component);

    /**
     * @brief Find and return the first component of type T attached to this object.
     * @tparam T A Component subclass decorated with the COMPONENT macro.
     * @return Raw pointer to the component, or nullptr if none is attached.
     */
    template<typename T, typename = typename std::enable_if_t<std::is_base_of_v<Component, T>>>
    T *GetComponent()
    {
        usize typeId = Component::StaticTypeId<T>();

        for (auto &component : m_components)
        {
            if (component->GetTypeId() == typeId)
            {
                return static_cast<T *>(component.get());
            }
        }

        return nullptr;
    }

    /**
     * @brief Re-parent this object in the scene hierarchy.
     * @param parent New parent node, or nullptr to make this a root object.
     * @return true on success.
     */
    bool SetParent(GameObject *parent);

    /// Returns the Scene that owns this object.
    Scene *GetScene();

    void SetActive(bool active);  ///< Show/hide this object (and stop Update calls).
    bool IsActive() const;        ///< Returns false if the object is hidden.

    /// @name Local Transform
    /// Position, rotation, and scale are all relative to the parent object.
    /// @{
    const vec3 &GetPosition() const;
    void        SetPosition(const vec3 &pos);

    void        SetRotation(const quat &rot);
    const quat &GetRotation() const;

    const vec3 &GetScale() const;
    void        SetScale(const vec3 &scale);
    /// @}

    /// @name World-Space Queries
    /// @{
    /**
     * @brief Compute the local transform matrix (TRS order).
     * @return A 4×4 matrix combining local position, rotation, and scale.
     */
    mat4 GetLocalTransform() const;

    /**
     * @brief Compute the world transform by walking up the parent chain.
     * @return Product of all ancestor local transforms × this local transform.
     */
    mat4 GetWorldTransform() const;

    /**
     * @brief Extract the world-space position from GetWorldTransform().
     * @return The translation column of the world matrix.
     */
    vec3 GetWorldPosition() const;
    /// @}

    void SetWorldPosition(const vec3 &pos);
    quat GetWorldRotation();
    void SetWorldRotation(const quat &rot);

    /**
     * @brief Load a GLTF / GLB file and build a matching GameObject hierarchy.
     *
     * Each GLTF node becomes a child GameObject. Meshes get a MeshComponent,
     * animations get an AnimationComponent attached to the root.
     *
     * @param path Filesystem path to the .gltf or .glb file.
     * @return Root of the loaded hierarchy (heap-allocated, caller takes ownership).
     */
    static GameObject *LoadGLTF(const std::string &path, Scene *scene);

    /**
     * @brief Depth-first search for a child with the given name.
     * @param name Exact name to search for (case-sensitive).
     * @return Pointer to the first matching descendant, or nullptr.
     */
    GameObject *FindChildByName(const std::string &name);

    /// Editor access: iterate immediate children (non-owning).
    const std::vector<std::unique_ptr<GameObject>> &GetChildren() const { return m_children; }

    /// Editor access: iterate attached components (non-owning).
    const std::vector<std::unique_ptr<Component>> &GetComponents() const { return m_components; }

protected:
    GameObject() = default;

private:
    std::vector<std::unique_ptr<GameObject>> m_children;    ///< Owned child nodes.
    std::vector<std::unique_ptr<Component>>  m_components;  ///< Owned component list.

    str         m_name;               ///< Display name (used for lookup and logging).
    GameObject *m_parent  = nullptr;  ///< Non-owning back-pointer to parent node.
    Scene      *m_scene   = nullptr;  ///< Non-owning back-pointer to owning Scene.
    bool        m_isAlive = true;     ///< Cleared by MarkForDestroy(); Scene prunes dead objects.

    vec3 m_position = vec3(0.0f);                    ///< Local position relative to parent.
    quat m_rotation = quat(1.0f, 0.0f, 0.0f, 0.0f);  ///< Local rotation as a unit quaternion.
    vec3 m_scale    = vec3(1.0f);                    ///< Local scale per axis.
    bool m_active   = true;                          ///< When false, Update() and rendering are skipped.

    friend class Scene;
};

#define GAMEOBJECT(ObjectClass) \
public: \
    static void Register() \
    { \
        COA::GameObjectFactory::GetInstance().RegisterObject<ObjectClass>(std::string(#ObjectClass)); \
    }

}  // namespace COA
