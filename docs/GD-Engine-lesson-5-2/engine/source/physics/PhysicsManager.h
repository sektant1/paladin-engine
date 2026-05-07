#pragma once

#include <memory>

class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;

namespace eng
{
    class RigidBody;

    class PhysicsManager
    {
    public:
        PhysicsManager();
        ~PhysicsManager();

        void Init();
        void Update(float deltaTime);

        void AddRigidBody(RigidBody* body);
        void RemoveRigidBody(RigidBody* body);

        btDiscreteDynamicsWorld* GetWorld();

    private:
        std::unique_ptr<btBroadphaseInterface> m_broadphase;
        std::unique_ptr<btDefaultCollisionConfiguration> m_collisionConfig;
        std::unique_ptr<btCollisionDispatcher> m_dispatcher;
        std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
        std::unique_ptr<btDiscreteDynamicsWorld> m_world;
    };
}