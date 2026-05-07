#include "physics/PhysicsManager.h"
#include "physics/RigidBody.h"
#include "physics/CollisionObject.h"

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

namespace eng
{
    PhysicsManager::PhysicsManager()
    {
    }

    PhysicsManager::~PhysicsManager()
    {
    }

    void PhysicsManager::Init()
    {
        m_broadphase = std::make_unique<btDbvtBroadphase>();
        m_collisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
        m_dispatcher = std::make_unique<btCollisionDispatcher>(m_collisionConfig.get());
        m_solver = std::make_unique<btSequentialImpulseConstraintSolver>();
        m_world = std::make_unique<btDiscreteDynamicsWorld>(
            m_dispatcher.get(), m_broadphase.get(), m_solver.get(), m_collisionConfig.get()
        );

        m_world->setGravity(btVector3(0, -9.81f, 0));
    }

    void PhysicsManager::Update(float deltaTime)
    {
        const btScalar fixedTimeStep = 1.0f / 60.0f;
        const int maxSubsteps = 4;
        m_world->stepSimulation(deltaTime, maxSubsteps, fixedTimeStep);

        // process collisions
        auto dispatcher = m_world->getDispatcher();
        const auto numManifolds = dispatcher->getNumManifolds();
        for (int i = 0; i < numManifolds; ++i)
        {
            auto manifold = dispatcher->getManifoldByIndexInternal(i);
            if (!manifold)
            {
                continue;
            }

            auto bodyA = reinterpret_cast<CollisionObject*>(manifold->getBody0()->getUserPointer());
            auto bodyB = reinterpret_cast<CollisionObject*>(manifold->getBody1()->getUserPointer());

            if (!bodyA || !bodyB)
            {
                continue;
            }

            const auto numContacts = manifold->getNumContacts();
            for (int j = 0; j < numContacts; ++j)
            {
                const auto& point = manifold->getContactPoint(j);
                const glm::vec3 pos(
                    point.m_positionWorldOnB.x(),
                    point.m_positionWorldOnB.y(),
                    point.m_positionWorldOnB.z());
                const glm::vec3 norm(
                    point.m_normalWorldOnB.x(),
                    point.m_normalWorldOnB.y(),
                    point.m_normalWorldOnB.z());

                bodyA->DispatchContactEvent(bodyB, pos, norm);
                bodyB->DispatchContactEvent(bodyA, pos, norm);
            }
        }
    }

    void PhysicsManager::AddRigidBody(RigidBody* body)
    {
        if (!body || !m_world)
        {
            return;
        }

        if (auto rigidBody = body->GetBody())
        {
            m_world->addRigidBody(rigidBody, btBroadphaseProxy::StaticFilter,
                btBroadphaseProxy::AllFilter);
            body->SetAddedToWorld(true);
        }
    }

    void PhysicsManager::RemoveRigidBody(RigidBody* body)
    {
        if (!body || !m_world)
        {
            return;
        }

        if (auto rigidBody = body->GetBody())
        {
            m_world->removeRigidBody(rigidBody);
            body->SetAddedToWorld(false);
        }
    }

    btDiscreteDynamicsWorld* PhysicsManager::GetWorld()
    {
        return m_world.get();
    }
}