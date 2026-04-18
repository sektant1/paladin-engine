#include "scene/GameObject.h"

#include <glm/ext/matrix_transform.hpp>

#include "Log.h"

namespace ENG
{

void GameObject::Update(f32 deltaTime)
{
    for (auto &component : m_components) {
        component->Update(deltaTime);
    }

    for (auto it = m_children.begin(); it != m_children.end();) {
        if ((*it)->IsAlive()) {
            (*it)->Update(deltaTime);
            ++it;
        } else {
            it = m_children.erase(it);
        }
    }
}

void GameObject::AddComponent(Component *component)
{
    if (!component) {
        LOG_ERROR("AddComponent called with nullptr on '%s'", m_name.c_str());
        return;
    }
    m_components.emplace_back(component);
    component->m_owner = this;
    LOG_INFO("Component added to '%s' (total=%zu)", m_name.c_str(), m_components.size());
}

const vec3 &GameObject::GetPosition() const
{
    return m_position;
}

void GameObject::SetPosition(const vec3 &pos)
{
    m_position = pos;
}

const vec3 &GameObject::GetRotation() const
{
    return m_rotation;
}

void GameObject::SetRotation(const vec3 &rot)
{
    m_rotation = rot;
}

const vec3 &GameObject::GetScale() const
{
    return m_scale;
}

void GameObject::SetScale(const vec3 &pos)
{
    m_scale = pos;
}

mat4 GameObject::GetLocalTransform() const
{
    mat4 mat = mat4(1.0F);

    // Translation
    mat = translate(mat, m_position);

    // Rotation
    mat = rotate(mat, m_rotation.x, vec3(1.0f, 0.0f, 0.0f));  // X Axis
    mat = rotate(mat, m_rotation.y, vec3(0.0f, 1.0f, 0.0f));  // Y Axis
    mat = rotate(mat, m_rotation.z, vec3(0.0f, 0.0f, 1.0f));  // Z Axis

    // Scale
    mat = scale(mat, m_scale);

    return mat;
}

mat4 GameObject::GetWorldTransform() const
{
    if (m_parent) {
        return m_parent->GetWorldTransform() * GetLocalTransform();
    } else {
        return GetLocalTransform();
    }
}

const str &GameObject::GetName() const
{
    return m_name;
}

void GameObject::SetName(const str &name)
{
    m_name = name;
}

GameObject *GameObject::GetParent()
{
    return m_parent;
}

bool GameObject::IsAlive() const
{
    return m_isAlive;
}

void GameObject::MarkForDestroy()
{
    LOG_INFO("GameObject '%s' marked for destroy", m_name.c_str());
    m_isAlive = false;
}

}  // namespace ENG
