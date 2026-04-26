#pragma once

#include <vector>

#include "Common.h"
#include "Types.h"

namespace mnd
{

class IContactListener;

enum class CollisionObjectType
{
    RigidBody,
    KinematicCharacterController
};

class CollisionObject
{
 public:
    CollisionObjectType GetCollisionObjectType();

    void AddContactListener(IContactListener *listener);
    void RemoveContactListener(IContactListener *listener);

 protected:
    void DispatchContactEvent(CollisionObject *obj, const vec3 &pos, const vec3 &norm);

    CollisionObjectType             m_collisionObjType;
    std::vector<IContactListener *> m_contactListeners;

    friend class PhysicsManager;
};

class IContactListener
{
 public:
    virtual void OnContact(CollisionObject *obj, const vec3 &pos, const vec3 &norm) = 0;
};

}  // namespace mnd
