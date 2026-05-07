#include "physics/CollisionObject.h"

#include <algorithm>

namespace mnd
{

CollisionObjectType CollisionObject::GetCollisionObjectType()
{
    return m_collisionObjType;
}

void CollisionObject::AddContactListener(IContactListener *listener)
{
    if (listener == nullptr)
    {
        return;
    }
    if (std::find(m_contactListeners.begin(), m_contactListeners.end(), listener) != m_contactListeners.end())
    {
        return;
    }
    m_contactListeners.push_back(listener);
}

void CollisionObject::RemoveContactListener(IContactListener *listener)
{
    auto iter = std::find(m_contactListeners.begin(), m_contactListeners.end(), listener);
    if (iter != m_contactListeners.end())
    {
        m_contactListeners.erase(iter);
    }
}

void CollisionObject::DispatchContactEvent(CollisionObject *obj, const vec3 &pos, const vec3 &norm)
{
    auto listeners = m_contactListeners;
    for (auto listener : listeners)
    {
        if (listener
            && std::find(m_contactListeners.begin(), m_contactListeners.end(), listener) != m_contactListeners.end())
        {
            listener->OnContact(obj, pos, norm);
        }
    }
}

}  // namespace mnd
