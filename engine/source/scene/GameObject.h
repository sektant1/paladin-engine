#pragma once
#include <memory>
#include <vector>

#include "Types.h"
#include "scene/Component.h"

namespace ENG
{
class Scene;

class GameObject
{
public:
    virtual ~GameObject() = default;
    virtual void Update(f32 deltaTime);
    const str   &GetName() const;
    void         SetName(const str &name);
    GameObject  *GetParent();
    bool         IsAlive() const;
    void         MarkForDestroy();

    void AddComponent(Component *component);

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

    bool   SetParent(GameObject *parent);
    Scene *GetScene();

    void SetActive(bool active);
    bool IsActive() const;

    const vec3 &GetPosition() const;
    void        SetPosition(const vec3 &pos);

    void        SetRotation(const quat &rot);
    const quat &GetRotation() const;

    const vec3 &GetScale() const;
    void        SetScale(const vec3 &pos);

    mat4 GetLocalTransform() const;
    mat4 GetWorldTransform() const;

    vec3 GetWorldPosition() const;

    static GameObject *LoadGLTF(const std::string &path);

    GameObject *FindChildByName(const std::string &name);

protected:
    GameObject() = default;

private:
    std::vector<std::unique_ptr<GameObject>> m_children;
    std::vector<std::unique_ptr<Component>>  m_components;

    str         m_name;
    GameObject *m_parent  = nullptr;
    Scene      *m_scene   = nullptr;
    bool        m_isAlive = true;

    vec3 m_position = vec3(0.0f);
    quat m_rotation = quat(1.0f, 0.0f, 0.0f, 0.0f);
    vec3 m_scale    = vec3(1.0f);
    bool m_active   = true;

    friend class Scene;
};

}  // namespace ENG
