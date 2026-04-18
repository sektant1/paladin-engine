#pragma once
#include <memory>
#include <vector>

#include "Types.h"
#include "scene/Component.h"

namespace ENG
{

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

        for (auto &component : m_components) {
            if (component->GetTypeId() == typeId) {
                return static_cast<T *>(component.get());
            }
        }

        return nullptr;
    }

    const vec3 &GetPosition() const;
    void        SetPosition(const vec3 &pos);

    void        SetRotation(const vec3 &rot);
    const vec3 &GetRotation() const;

    const vec3 &GetScale() const;
    void        SetScale(const vec3 &pos);

    mat4 GetLocalTransform() const;
    mat4 GetWorldTransform() const;

protected:
    GameObject() = default;

private:
    std::vector<std::unique_ptr<GameObject>> m_children;
    std::vector<std::unique_ptr<Component>>  m_components;

    str         m_name;
    GameObject *m_parent  = nullptr;
    bool        m_isAlive = true;

    vec3 m_position = vec3(0.0f);
    vec3 m_rotation = vec3(0.0f);
    vec3 m_scale    = vec3(1.0f);

    friend class Scene;
};

}  // namespace ENG
