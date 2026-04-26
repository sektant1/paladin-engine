#include "physics/CollisionObject.h"

namespace mnd
{

CollisionObjectType CollisionObject::GetCollisionObjectType()
{
    return m_collisionObjType;
}

void CollisionObject::AddContactListener(IContactListener *listener)
{
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
    for (auto listener : m_contactListeners)
    {
        if (listener)
        {
            listener->OnContact(obj, pos, norm);
        }
    }
}

}  // namespace mnd
