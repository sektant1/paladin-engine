#pragma once

#include "Common.h"
#include <vector>

namespace eng
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

        void AddContactListener(IContactListener* listener);
        void RemoveContactListener(IContactListener* listener);

    protected:
        void DispatchContactEvent(
            CollisionObject* obj,
            const glm::vec3& pos,
            const glm::vec3& norm);

        CollisionObjectType m_collisionObjectType;
        std::vector<IContactListener*> m_contactListeners;

        friend class PhysicsManager;
    };

    class IContactListener
    {
    public:
        virtual void OnContact(
            CollisionObject* obj,
            const glm::vec3& pos,
            const glm::vec3& norm) = 0;
    };
}